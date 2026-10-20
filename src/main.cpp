
#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace jb {

struct State {
    bool pending = false;
    int button = 1;
    bool isPlayer1 = true;
    int ticksRemaining = 0;

    bool releaseQueued = false;
    int releaseCountdown = -1;
};

inline State g;

inline bool getBool(char const* key) {
    return Mod::get()->getSettingValue<bool>(key);
}

inline int getInt(char const* key) {
    return static_cast<int>(
        Mod::get()->getSettingValue<int64_t>(key)
    );
}

inline bool active(GJBaseGameLayer* layer) {
    if (!getBool("enabled"))
        return false;

    auto* pl = PlayLayer::get();
    if (!pl || static_cast<GJBaseGameLayer*>(pl) != layer)
        return false;

    if (getBool("practice-only") && !pl->m_isPracticeMode)
        return false;

    return true;
}

} // namespace jb

class $modify(JumpBufferGameLayer, GJBaseGameLayer) {
    bool isHoldMode(bool secondPlayer) {
        auto* player = secondPlayer ? m_player2 : m_player1;
        if (!player)
            player = m_player1;

        if (!player)
            return false;

        return player->m_isShip
            || player->m_isBird
            || player->m_isDart
            || player->m_isRobot
            || player->m_isSpider
            || player->m_isSwing;
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        if (button != 1 || !jb::active(this)) {
            GJBaseGameLayer::handleButton(
                down, button, isPlayer1
            );
            return;
        }

        // Preserve the normal release behavior unless a buffered
        // press is waiting to be delivered.
        if (!down) {
            if (jb::g.pending) {
                jb::g.releaseQueued = true;
                return;
            }

            if (jb::g.releaseCountdown >= 0)
                return;

            GJBaseGameLayer::handleButton(
                false, button, isPlayer1
            );
            return;
        }

        // Do not stack buffered presses.
        if (jb::g.pending)
            return;

        bool secondPlayer = !isPlayer1;
        bool hold = isHoldMode(secondPlayer);

        if (hold && !jb::getBool("hold-modes")) {
            GJBaseGameLayer::handleButton(
                true, button, isPlayer1
            );
            return;
        }

        int window = jb::getInt("ticks");
        if (window < 0)
            window = 0;

        // No prediction is performed inside the input callback.
        // A positive window delays the input by that many game ticks.
        if (window == 0) {
            GJBaseGameLayer::handleButton(
                true, button, isPlayer1
            );
            return;
        }

        jb::g.pending = true;
        jb::g.button = button;
        jb::g.isPlayer1 = isPlayer1;
        jb::g.ticksRemaining = window;
        jb::g.releaseQueued = false;
        jb::g.releaseCountdown = -1;
    }

    void processCommands(
        float dt,
        bool isHalfTick,
        bool isLastTick
    ) {
        if (jb::g.pending && isLastTick) {
            --jb::g.ticksRemaining;

            if (jb::g.ticksRemaining <= 0) {
                jb::g.pending = false;

                GJBaseGameLayer::handleButton(
                    true,
                    jb::g.button,
                    jb::g.isPlayer1
                );

                if (jb::g.releaseQueued) {
                    jb::g.releaseQueued = false;
                    jb::g.releaseCountdown = 2;
                }
            }
        } else if (
            !jb::g.pending
            && jb::g.releaseCountdown >= 0
            && isLastTick
        ) {
            --jb::g.releaseCountdown;

            if (jb::g.releaseCountdown < 0) {
                GJBaseGameLayer::handleButton(
                    false,
                    jb::g.button,
                    jb::g.isPlayer1
                );

                jb::g.releaseCountdown = -1;
            }
        }

        GJBaseGameLayer::processCommands(
            dt, isHalfTick, isLastTick
        );
    }
};

class $modify(JumpBufferPlayLayer, PlayLayer) {
    void resetLevel() {
        jb::g = {};
        PlayLayer::resetLevel();
    }
};

$on_mod(Loaded) {
    log::info("[JumpBuffer] Non-blocking input buffer loaded");
}
