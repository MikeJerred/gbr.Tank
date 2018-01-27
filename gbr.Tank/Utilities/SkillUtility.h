#pragma once

#include <GWCA/GWCA.h>
#include <GWCA/Managers/SkillbarMgr.h>

namespace gbr::Tank::Utilities {
    class SkillUtility {
    public:
        static bool TryUseSkill(GW::Constants::SkillID skillId, DWORD targetId);
    };
}