#include "PlayerAnimationManager.h"
#include "EquipeTypes.h"
#include "playerInfo.h"
#include "stbResourceManager.h"
#include "Util.h"
#include <fstream>

#define PLAYER_ANIM_PATH "Data/Character/"

#define M_RESOURCEMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
namespace fs = std::filesystem;


bool PlayerAnimationManager::Init()
{
    if (!PreLoadAll()) return false;

    return true;
}

bool PlayerAnimationManager::PreLoadAll()
{
    fs::path root = PLAYER_ANIM_PATH;
    std::error_code ec;

    if (!fs::exists(root, ec) || !fs::is_directory(root, ec))
    {
        OutputDebugStringA(("[PlayerAnimationManager] animation path not found: "
            + root.string() + "\n").c_str());
        return false;
    }

    for (const auto& entry : fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, ec))
    {
        if (ec)
        {
            OutputDebugStringA(("[PlayerAnimationManager] filesystem error: "
                + ec.message() + "\n").c_str());
            break;
        }

        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() != ".json")
            continue;

        PlayerAnimationSet animSet{};
        if (!LoadJsonFile(entry.path().string(), animSet))
            return false;

        m_animationSets.emplace(animSet.key, animSet);
    }

    return true;
}

bool PlayerAnimationManager::LoadJsonFile(const std::string& path, PlayerAnimationSet& animData)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
    }
    catch (const nlohmann::json::parse_error&) {
        return false;
    }
    if (j.is_null()) return false;

    animData.key.jobType = PlayerTypeUtil::StringToJobType(j.at("job").get<std::string>());
    animData.key.weaponType = Weapon::StringToWeaponType(j.value("weapon", "Unknown"));
  
    m_debugMsg = "jobType : " + j.at("job").get<std::string>() + "weapon : " + j.value("weapon", "Unknown") + "\n";
    OutputDebugStringA(m_debugMsg.c_str());
    const auto& anims = j.at("animations");

    // get은 필수 값 없어서는 안되는 값
    // value는 없어도 기본값으로 동작 가능할 때 사용
    for (auto& [animName, animJson] : anims.items())
    {
        PlayerAnimationInfo info;

        info.animName = animName;
        info.path = animJson.at("path").get<std::string>();
        info.framePrefix = animJson.at("frame_prefix").get<std::string>();
        info.frameCount = animJson.value("frame_count", 1);
        info.delayMs = animJson.value("delay_ms", 0.1f);

        if (animJson.contains("frame_offsets"))
        {
            for (const auto& offsetJson : animJson["frame_offsets"])
            {
                stb::math::Vector2 frameOffset;
                frameOffset.x = offsetJson.value("x", 0.0f);
                frameOffset.y = offsetJson.value("y", 0.0f);

                info.frameOffsets.push_back(frameOffset);
            }
        }

        if (animJson.contains("event"))
        {
            const auto& events = animJson.at("event");

            info.animationEvent.start = events.value("start", "");
            info.animationEvent.complete = events.value("complete", "");
            info.animationEvent.end = events.value("end", "");
        }

        animData.animations.emplace_back(info);
    }

    const auto& render = j.at("render");
    animData.renderInfo.width = render.value("width", 0);
    animData.renderInfo.height = render.value("height", 0);

    const auto& origin = render.at("origin");
    animData.renderInfo.origin.x = origin.value("x", 0.0f);
    animData.renderInfo.origin.y = origin.value("y", 0.0f);

    return true;
}

bool PlayerAnimationManager::SetupPlayerAnimations(stb::Player* player, JobType jobtype, WeaponType weaponType)
{
    if (player == nullptr)
        return false;

    stb::Animator* animator = player->GetComponent<stb::Animator>();
    if (animator == nullptr)
        animator = player->AddComponent<stb::Animator>();

    const PlayerAnimationSet* animSet = FindAnimationSet(jobtype, weaponType);
    if (animSet == nullptr)
        return false;

    for (const PlayerAnimationInfo& info : animSet->animations)
    {
        std::vector<stb::Texture*> frames;
        for (int i = 0; i < info.frameCount; ++i)
        {
            std::wstring key =
                utils::StringToWString(info.path + "/" + info.framePrefix + std::to_string(i));

            stb::Texture* tex = M_RESOURCEMANAGER->Find<stb::Texture>(key);
            if (tex != nullptr)
                frames.emplace_back(tex);
        }
       

        if (frames.empty())
        {
            std::string msg = "[PlayerAnimationManager] No frame files found for animation: " + info.animName + " (path=" + info.path + ")\n";
            OutputDebugStringA(msg.c_str());
            // 다음 애니메이션으로 넘어감
            continue;
        }

        // origin 결정: JSON에 origin이 (0,0)인 경우 프레임 첫 이미지 중앙을 기본 origin으로 사용
        stb::math::Vector2 originToUse = animSet->renderInfo.origin;
        if (originToUse.x == 0.0f && originToUse.y == 0.0f)
        {
            stb::Texture* first = frames.front();
            originToUse.x = static_cast<float>(first->GetWidth()) * 0.5f;
            originToUse.y = static_cast<float>(first->GetHeight()) * 0.5f;
        }

        animator->CreateFrameAnimation(
            utils::StringToWString(info.animName),
            frames,
            originToUse,
            info.frameOffsets,
            info.delayMs
        );

        stb::Animator::EventNames eventNames;

        eventNames.startEventName = utils::StringToWString(info.animationEvent.start);
        eventNames.completeEventName = utils::StringToWString(info.animationEvent.complete);
        eventNames.endEventName = utils::StringToWString(info.animationEvent.end);

        animator->SetAnimationEventNames(utils::StringToWString(info.animName), eventNames);
       
    }
    BindPlayerAnimationEvents(player, animator);
    return true;
}

bool PlayerAnimationManager::SetupOtherPlayerAnimations(stb::OtherPlayer* otherPlayer, JobType jobtype, WeaponType weaponType)
{
    {
        std::string msg = "SetupOtherPlayerAnimation jobtype : " + std::to_string(static_cast<int>(jobtype)) + "weaponType :" + std::to_string(static_cast<int>(weaponType)) + "\n";
        OutputDebugStringA(msg.c_str());
    }
    if (otherPlayer == nullptr)
        return false;

    stb::Animator* animator = otherPlayer->GetComponent<stb::Animator>();
    if (animator == nullptr)
        animator = otherPlayer->AddComponent<stb::Animator>();

    const PlayerAnimationSet* animSet = FindAnimationSet(jobtype, weaponType);
    if (animSet == nullptr)
        return false;

    int createdCount = 0;

    for (const PlayerAnimationInfo& info : animSet->animations)
    {
        std::vector<stb::Texture*> frames;
        for (int i = 0; i < info.frameCount; ++i)
        {
            std::wstring key =
                utils::StringToWString(info.path + "/" + info.framePrefix + std::to_string(i));

            stb::Texture* tex = M_RESOURCEMANAGER->Find<stb::Texture>(key);
            if (tex != nullptr)
                frames.emplace_back(tex);
        }


        if (frames.empty())
        {
            std::string msg = "[PlayerAnimationManager] No frame files found for animation: " + info.animName + " (path=" + info.path + ")\n";
            OutputDebugStringA(msg.c_str());
            // 다음 애니메이션으로 넘어감
            continue;
        }

        // origin 결정: JSON에 origin이 (0,0)인 경우 프레임 첫 이미지 중앙을 기본 origin으로 사용
        stb::math::Vector2 originToUse = animSet->renderInfo.origin;
        if (originToUse.x == 0.0f && originToUse.y == 0.0f)
        {
            stb::Texture* first = frames.front();
            originToUse.x = static_cast<float>(first->GetWidth()) * 0.5f;
            originToUse.y = static_cast<float>(first->GetHeight()) * 0.5f;
        }

        animator->CreateFrameAnimation(
            utils::StringToWString(info.animName),
            frames,
            originToUse,
            info.frameOffsets,
            info.delayMs
        );

        stb::Animator::EventNames eventNames;

        eventNames.startEventName = utils::StringToWString(info.animationEvent.start);
        eventNames.completeEventName = utils::StringToWString(info.animationEvent.complete);
        eventNames.endEventName = utils::StringToWString(info.animationEvent.end);

        animator->SetAnimationEventNames(utils::StringToWString(info.animName), eventNames);
        ++createdCount;

    }
    if (createdCount == 0)
        return false;

    BindOtherPlayerAnimationEvents(otherPlayer, animator);
    return true;
}



void PlayerAnimationManager::BindPlayerAnimationEvents(stb::Player* player, stb::Animator* animator)
{
   // 나중에 이벤트들 여기로 옮겨야함

    if (player == nullptr || animator == nullptr)
        return;

    animator->RegisterEvent(L"PlayerAttackEnd", [player]()
        {
            OutputDebugStringA("PlayerAttackEnd event called\n");

            if (player == nullptr)
                return;

            if (player->GetState() != PlayerState::Attack)
                return;

            player->SetState(PlayerState::Idle);
        });
}

void PlayerAnimationManager::BindOtherPlayerAnimationEvents(stb::OtherPlayer* otherPlayer, stb::Animator* animator)
{
    if (otherPlayer == nullptr || animator == nullptr)
        return;

    animator->RegisterEvent(L"PlayerAttackEnd", [otherPlayer]()
        {
            OutputDebugStringA("PlayerAttackEnd event called\n");

            if (otherPlayer == nullptr)
                return;

            if (otherPlayer->GetState() != PlayerState::Attack)
                return;

            otherPlayer->SetState(PlayerState::Idle);
        });
}

const PlayerAnimationSet* PlayerAnimationManager::FindAnimationSet(JobType jobtype, WeaponType weaponType) const
{
    PlayerAnimationKey key{};

    key.jobType = jobtype;
    key.weaponType = weaponType;
    auto it = m_animationSets.find(key);

    if (it == m_animationSets.end())
    {
        std::string temp;
        temp = "Can't find jobType : " + std::to_string(static_cast<int>(jobtype)) + " WeaponType : " + std::to_string(static_cast<int>(weaponType)) + "\n";
        OutputDebugStringA(temp.c_str());
        return nullptr;
    }

    return &(it->second);
}
