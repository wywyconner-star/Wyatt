#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

using namespace geode::prelude;

// ============================================================================
// DECORATION DATA STRUCTURES & MAPPING EXPLANATION
// ============================================================================
//
// How Block-to-Decoration Mapping Works in Geometry Dash:
// 1. Every object in Geometry Dash has a unique integer Object ID (`m_objectID`).
//    For example:
//      - 1   = Standard 1x1 Default Block (30x30 units in Cocos2d space)
//      - 8   = Standard Hazard Spike
//      - 83  = Grid Block / Pillar Block
//      - 503 = Standard Corner Glow
//      - 88  = Metal Sawblade
//
// 2. One full grid tile in the GD Editor is 30.0f x 30.0f units.
//    Therefore, relative offsets (`offsetX`, `offsetY`) are measured from the
//    center of the selected source block:
//      - (0.0f,  30.0f)  -> 1 full tile directly ABOVE the block
//      - (0.0f, -30.0f)  -> 1 full tile directly BELOW the block
//      - (0.0f,  15.0f)  -> Sitting flush on the TOP edge of the block
//
// 3. To map a source structural block to decorations:
//    We define a `DecoRule` containing a list of `DecoAttachment` items.
//    Each theme maps a source `blockID` (or `0` as a universal fallback for
//    any structural block) to a vector of `DecoAttachment` structs.
// ============================================================================

enum class DecoTheme {
    Hell,
    Space,
    Candy,
    RealLife
};

struct DecoAttachment {
    int decoObjectID;       // The GD Object ID to spawn
    CCPoint offset;         // Offset in GD units (30.0f = 1 grid block)
    float rotation = 0.0f;  // Rotation in degrees
    float scale = 1.0f;     // Scale multiplier
    int zOrderOffset = 1;   // Relative Z-order (+1 = in front, -1 = behind)
    int zLayer = 0;         // Editor layer
    ccColor3B tintColor = {255, 255, 255}; // Visual color override
};

// Maps a Source Structural Block ID -> List of Decoration Attachments
// Key `0` is treated as a "wildcard" applied to any recognized structural block.
using ThemeRuleMap = std::unordered_map<int, std::vector<DecoAttachment>>;

namespace DecoPresets {

    // Recognized structural block IDs that are valid candidates for decoration
    static const std::unordered_set<int> kStructuralBlockIDs = {
        1, 2, 3, 4, 6, 7,       // Classic 1.0 blocks & slabs
        83, 84,                 // Pillar / Grid blocks
        143, 144, 145,          // Metallic / Industrial blocks
        192, 193, 194,          // Stone / Brick blocks
        263, 264,               // Outlined blocks
        1196, 1197, 1198        // 2.0/2.1 Solid base blocks
    };

    // ------------------------------------------------------------------------
    // 1. HELL THEME (Crimson bricks, spikes, skulls, fire, dark saws, red glow)
    // ------------------------------------------------------------------------
    inline ThemeRuleMap getHellTheme() {
        ThemeRuleMap rules;

        // Universal decorations applied to any structural block (Key = 0)
        rules[0] = {
            // Inner dark brick fill overlay
            { 192, {0.0f, 0.0f},   0.0f, 1.0f,  1, 0, {160, 20, 20} },
            // Top jagged spike hazard (ID 8 = standard spike)
            { 8,   {0.0f, 30.0f},  0.0f, 1.0f,  2, 0, {220, 30, 30} },
            // Mini side spikes for aggressive hell Spike-bush framing
            { 39,  {-10.0f, 22.0f}, -25.0f, 0.8f, 1, 0, {120, 10, 10} },
            { 39,  { 10.0f, 22.0f},  25.0f, 0.8f, 1, 0, {120, 10, 10} },
            // Fire / Lava detail on top surface (ID 917 = Fireball / flame deco)
            { 917, {0.0f, 15.0f},  0.0f, 0.85f, 3, 0, {255, 100, 0} },
            // Skull ornament inside the block (ID 406 = Skull deco)
            { 406, {0.0f, 0.0f},   0.0f, 0.65f, 4, 0, {255, 200, 200} }
        };

        // Specific override for Default Block (ID 1): Add background sawblade
        rules[1] = {
            // Heavy background hell sawblade (ID 88 = Sawblade)
            { 88,  {0.0f, 0.0f},   0.0f, 0.75f, -2, 0, {90, 0, 0} }
        };

        return rules;
    }

    // ------------------------------------------------------------------------
    // 2. SPACE / COSMIC THEME (Nebula glow, stars, planets, floating crystals)
    // ------------------------------------------------------------------------
    inline ThemeRuleMap getSpaceTheme() {
        ThemeRuleMap rules;

        rules[0] = {
            // Deep space metallic core overlay
            { 143, {0.0f, 0.0f},    0.0f,  1.0f,  1, 0, {35, 25, 85} },
            // Floating 4-point Cosmic Star (ID 725 = Star decoration)
            { 725, {-12.0f, 26.0f}, 15.0f, 0.65f, 3, 0, {130, 220, 255} },
            { 725, { 14.0f, 34.0f}, 45.0f, 0.40f, 3, 0, {255, 180, 255} },
            // Orbiting mini planet / ring sphere (ID 1705 = Energy orb / sphere deco)
            { 1705, {0.0f, 0.0f},   0.0f,  0.55f, 2, 0, {90, 255, 220} },
            // Space crystal shards protruding from bottom (ID 211 = Crystal spike)
            { 211, {0.0f, -30.0f}, 180.0f, 0.90f, 1, 0, {170, 80, 255} }
        };

        // Specific addition for Grid Block (ID 83): Tech energy ring
        rules[83] = {
            { 1706, {0.0f, 0.0f},   0.0f,  1.15f, -1, 0, {0, 255, 255} }
        };

        return rules;
    }

    // ------------------------------------------------------------------------
    // 3. CANDY / SWEETLAND THEME (Pastel frosting, swirls, hearts, gumdrops)
    // ------------------------------------------------------------------------
    inline ThemeRuleMap getCandyTheme() {
        ThemeRuleMap rules;

        rules[0] = {
            // Pastel pink wafer base
            { 263, {0.0f, 0.0f},   0.0f, 1.0f,  1, 0, {255, 165, 215} },
            // Frosting / Cloud drip along the top edge (ID 130 = Cloud / Fluffy top)
            { 130, {0.0f, 15.0f},  0.0f, 0.65f, 3, 0, {255, 245, 250} },
            // Lollipop Swirl center piece (ID 184 = Swirl wheel deco)
            { 184, {0.0f, 0.0f},   0.0f, 0.50f, 2, 0, {140, 240, 255} },
            // Candy Heart topper (ID 1585 = Heart / cute icon deco)
            { 1585, {0.0f, 28.0f}, 0.0f, 0.60f, 4, 0, {255, 95, 160} },
            // Pastel sprinkles (Small rotated stars)
            { 725, {-9.0f, 8.0f},  30.0f, 0.30f, 4, 0, {255, 255, 130} },
            { 725, { 9.0f, -6.0f}, -20.0f, 0.30f, 4, 0, {150, 255, 180} }
        };

        return rules;
    }

    // ------------------------------------------------------------------------
    // 4. REAL-LIFE / NATURE THEME (Brickwork, grass turf, hanging vines, timber)
    // ------------------------------------------------------------------------
    inline ThemeRuleMap getRealLifeTheme() {
        ThemeRuleMap rules;

        rules[0] = {
            // Earthy masonry / stone brick body (ID 193 = Detailed brick)
            { 193, {0.0f, 0.0f},   0.0f,  1.0f,  1, 0, {135, 115, 95} },
            // Lush grass blades sitting on the top surface (ID 40 = Grass / Bush edge)
            { 40,  {0.0f, 16.0f},  0.0f,  1.0f,  3, 0, {75, 185, 65} },
            // Wooden support beam trim (ID 144 = Structural beam)
            { 144, {0.0f, -10.0f}, 0.0f,  0.95f, 2, 0, {105, 70, 40} },
            // Hanging ivy / foliage below the block (ID 41 = Hanging vine / bush)
            { 41,  {0.0f, -20.0f}, 180.0f, 0.80f, 2, 0, {55, 145, 55} },
            // Small pebble / rock detail on top (ID 408 = Rock detail)
            { 408, {8.0f, 20.0f},  0.0f,  0.50f, 4, 0, {160, 160, 165} }
        };

        return rules;
    }

} // namespace DecoPresets


// ============================================================================
// EDITOR UI HOOK
// ============================================================================

class $modify(MyEditorUI, EditorUI) {
    struct Fields {
        DecoTheme m_activeTheme = DecoTheme::Hell;
    };

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

        // Load initial theme preference from mod.json settings
        this->syncThemeFromSettings();

        // Find the top-right undo/redo menu bar in EditorUI using Geode Node IDs
        if (auto menu = this->getChildByID("undo-menu")) {
            auto btnSprite = ButtonSprite::create(
                "Decorate",
                "bigFont.fnt",
                "GJ_button_04.png",
                0.6f
            );
            btnSprite->setScale(0.65f);

            auto decoBtn = CCMenuItemSpriteExtra::create(
                btnSprite,
                this,
                menu_selector(MyEditorUI::onAutoDecorate)
            );
            decoBtn->setID("auto-decorate-button"_spr);

            menu->addChild(decoBtn);
            menu->updateLayout();
        }

        return true;
    }

    // Reads the user's selected theme from Geode's Mod Settings
    void syncThemeFromSettings() {
        auto themeStr = Mod::get()->getSettingValue<std::string>("theme");
        if (themeStr == "Space") {
            m_fields->m_activeTheme = DecoTheme::Space;
        } else if (themeStr == "Candy") {
            m_fields->m_activeTheme = DecoTheme::Candy;
        } else if (themeStr == "RealLife") {
            m_fields->m_activeTheme = DecoTheme::RealLife;
        } else {
            m_fields->m_activeTheme = DecoTheme::Hell;
        }
    }

    // Helper that returns the rule map and ambient glow color for the active theme
    ThemeRuleMap getActiveThemeRules(ccColor3B& outGlowColor) {
        switch (m_fields->m_activeTheme) {
            case DecoTheme::Hell:
                outGlowColor = {255, 30, 0};
                return DecoPresets::getHellTheme();
            case DecoTheme::Space:
                outGlowColor = {90, 60, 255};
                return DecoPresets::getSpaceTheme();
            case DecoTheme::Candy:
                outGlowColor = {255, 140, 210};
                return DecoPresets::getCandyTheme();
            case DecoTheme::RealLife:
                outGlowColor = {120, 200, 90};
                return DecoPresets::getRealLifeTheme();
        }
        outGlowColor = {255, 255, 255};
        return DecoPresets::getHellTheme();
    }

    // Spawns a single decoration object relative to a source structural block
    GameObject* spawnDecoration(
        LevelEditorLayer* editorLayer,
        GameObject* sourceBlock,
        DecoAttachment const& spec
    ) {
        CCPoint basePos = sourceBlock->getPosition();
        CCPoint targetPos = ccp(basePos.x + spec.offset.x, basePos.y + spec.offset.y);

        GameObject* decoObj = editorLayer->createObject(spec.decoObjectID, targetPos, true);
        if (!decoObj) {
            return nullptr;
        }

        decoObj->setRotation(spec.rotation);
        decoObj->setScale(spec.scale);

        int baseZ = sourceBlock->m_zOrder != 0 ? sourceBlock->m_zOrder : 1;
        decoObj->m_zOrder = baseZ + spec.zOrderOffset;
        decoObj->m_editorLayer = sourceBlock->m_editorLayer + spec.zLayer;
        decoObj->setObjectColor(spec.tintColor);

        return decoObj;
    }

    // ========================================================================
    // CALLBACK: Triggered when the user clicks the "Decorate" button
    // ========================================================================
    void onAutoDecorate(CCObject* sender) {
        this->syncThemeFromSettings();

        LevelEditorLayer* editorLayer = m_editorLayer;
        if (!editorLayer) return;

        CCArray* selectedObjects = this->getSelectedObjects();
        if (!selectedObjects || selectedObjects->count() == 0) {
            Notification::create("Select structural blocks first!", NotificationIcon::Warning)->show();
            return;
        }

        std::vector<GameObject*> sourceBlocks;
        for (auto* obj : CCArrayExt<GameObject*>(selectedObjects)) {
            if (obj && DecoPresets::kStructuralBlockIDs.contains(obj->m_objectID)) {
                sourceBlocks.push_back(obj);
            }
        }

        if (sourceBlocks.empty()) {
            Notification::create(
                "No supported structural blocks in selection (Try IDs 1, 83, 143, 192).",
                NotificationIcon::Info
            )->show();
            return;
        }

        ccColor3B ambientGlowColor;
        ThemeRuleMap themeRules = this->getActiveThemeRules(ambientGlowColor);
        bool addGlow = Mod::get()->getSettingValue<bool>("enable-glow");

        CCArray* spawnedObjects = CCArray::create();

        for (GameObject* block : sourceBlocks) {
            int blockID = block->m_objectID;

            // 1. Apply universal rules for this theme (Key = 0)
            if (themeRules.contains(0)) {
                for (auto const& attachment : themeRules[0]) {
                    if (auto* created = this->spawnDecoration(editorLayer, block, attachment)) {
                        spawnedObjects->addObject(created);
                    }
                }
            }

            // 2. Apply block-specific rules if defined for this exact `m_objectID`
            if (themeRules.contains(blockID)) {
                for (auto const& attachment : themeRules[blockID]) {
                    if (auto* created = this->spawnDecoration(editorLayer, block, attachment)) {
                        spawnedObjects->addObject(created);
                    }
                }
            }

            // 3. Optional: Spawn ambient background glow (Object ID 505 = Large Soft Glow)
            if (addGlow) {
                DecoAttachment glowSpec {
                    .decoObjectID = 505,
                    .offset = {0.0f, 0.0f},
                    .rotation = 0.0f,
                    .scale = 1.75f,
                    .zOrderOffset = -5,
                    .zLayer = 0,
                    .tintColor = ambientGlowColor
                };
                if (auto* glowObj = this->spawnDecoration(editorLayer, block, glowSpec)) {
                    spawnedObjects->addObject(glowObj);
                }
            }
        }

        if (spawnedObjects->count() > 0) {
            this->selectObjects(spawnedObjects, true);
            this->updateButtons();

            auto themeName = Mod::get()->getSettingValue<std::string>("theme");
            Notification::create(
                fmt::format("Applied {} theme ({} objects spawned)!", themeName, spawnedObjects->count()),
                NotificationIcon::Success
            )->show();
        }
    }
};
