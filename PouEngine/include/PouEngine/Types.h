
#include <string>
#include <memory>
//#include <iostream>
#include <chrono>
#include <unordered_set>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_precision.hpp>

#ifndef   POUENGINE_TYPES
#define   POUENGINE_TYPES

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3



#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif


namespace pou
{

class SceneObject;
class SceneNode;
/**class SceneEntity;
class Light;
class ShadowCaster;**/

template<class AssetType> class AssetHandler;
class TextureAsset;
class SpriteSheetAsset;
class MaterialAsset;
class MeshAsset;
class FontAsset;
class SoundAsset;
class SoundBankAsset;
class SkeletonModelAsset;
//class CharacterModelAsset;

class Timer;

enum RendererName
{
    //Renderer_Default,
    Renderer_Scene,
    //Renderer_Instancing,
    Renderer_Ui,
};

enum RenderereOrder
{
    Renderer_First,
    Renderer_Middle,
    Renderer_Last,
    Renderer_Unique,
};

enum BlendMode
{
    BlendMode_None,
    BlendMode_Add,
    BlendMode_Alpha,
    BlendMode_Accu,
    BlendMode_Custom,
};

enum AssetLoadType
{
    LoadType_Now = 0,
    LoadType_InThread = 1,
};

enum AssetLoadSource
{
    LoadSource_None,
    LoadSource_File,
    LoadSource_Memory,
    LoadSource_Stream,
};

enum NotificationType
{
    NotificationType_SenderDestroyed,
    NotificationType_AssetLoaded,
    NotificationType_NodeMoved,
    NotificationType_NodeParentChanged,
    NotificationType_TransformChanged,
    NotificationType_TransformAskForUpdateModelMatrix,
    NotificationType_UpdateCmb,
    NotificationType_ModelChanged,
    NotificationType_TextureChanged,
    NotificationType_TextureIsAboutToChange,
    NotificationType_Ui_ButtonClicked,
    //NotificationType_UpdateShadow,
    NotificationType_Custom,
   // NotificationType_LightMoved,
};

enum LightType
{
    LightType_Omni,
    LightType_Directional,
    LightType_Spot,
};

//Could use flag
enum ShadowCastingType
{
    ShadowCasting_None,
    ShadowCasting_OnlyDirectional,
    ShadowCasting_OnlyDynamic,
    ShadowCasting_All,
};

enum ShadowVolumeType
{
    OneSidedShadow,
    TwoSidedShadow,
    MirroredTwoSidedShadow,
};


enum SkelAnimCmdType
{
    Unknown_Command,
    Move_To,
    Rotate_To,
    Scale_To,
    Color_To
};

enum CommandPoolName
{
    COMMANDPOOL_DEFAULT,
    COMMANDPOOL_SHORTLIVED, //fort short-lived command buffers
    COMMANDPOOL_TEXTURESLOADING, //for texture loading thread
    COMMANDPOOL_MESHESLOADING, //for meshes loading thread
    COMMANDPOOL_NBR_NAMES,
};

enum ConnectionStatus
{
    ConnectionStatus_Disconnected,
    ConnectionStatus_Connecting,
    ConnectionStatus_Challenging,
    ConnectionStatus_Connected,
    NBR_CONNECTIONSTATUS,
};

enum TextAlignType
{
    TextAlignType_Left,
    TextAlignType_Center, //Not completely implemented *yet*
    TextAlignType_Right, //Not completely implemented *yet*
};

typedef std::chrono::duration<double, std::chrono::seconds::period> Time;

typedef unsigned int AssetTypeId;
///typedef unsigned int NodeTypeId;
typedef unsigned int ObjectTypeId;
typedef unsigned int SoundTypeId;
typedef unsigned int FontTypeId;

typedef glm::vec4 Color;

typedef AssetHandler<TextureAsset>      TexturesHandler;
typedef AssetHandler<SpriteSheetAsset>  SpriteSheetsHandler;
typedef AssetHandler<MaterialAsset>     MaterialsHandler;
typedef AssetHandler<SkeletonModelAsset>SkeletonModelsHandler;
typedef AssetHandler<FontAsset>         FontsHandler;
typedef AssetHandler<SoundAsset>        SoundsHandler;
typedef AssetHandler<SoundBankAsset>    SoundBanksHandler;
//typedef AssetHandler<CharacterModelAsset>     CharacterModelsHandler;
typedef AssetHandler<MeshAsset>         MeshAssetsHandler;

const std::string emptyString;



struct IntPairHash {
  std::size_t operator()(const std::pair<uint32_t, uint32_t> &p) const {
    assert(sizeof(std::size_t)>=8);  //Ensure that std::size_t, the type of the hash, is large enough
    //Shift first integer over to make room for the second integer. The two are
    //then packed side by side.
    return (((uint64_t)p.first)<<32) | ((uint64_t)p.second);
  }
};

typedef size_t HashedString;

typedef std::unordered_set< std::pair<int,int>, IntPairHash > unordered_set_intpair;

}

#endif
