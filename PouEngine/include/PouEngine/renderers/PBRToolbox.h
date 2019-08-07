#ifndef PBRTOOLBOX_H
#define PBRTOOLBOX_H

#include "PouEngine/utils/Singleton.h"
#include "PouEngine/vulkanImpl/VulkanImpl.h"

namespace pou
{

class PBRToolbox : public Singleton<PBRToolbox>
{
    public:
        friend class Singleton<PBRToolbox>;

        static VFramebufferAttachment getBrdflut();

        static VFramebufferAttachment generateFilteredEnvMap(VTexture src);

        void cleanup();

    protected:
        PBRToolbox();
        virtual ~PBRToolbox();

        bool generateBrdflut();


    private:
        VFramebufferAttachment m_brdflutAttachement;

    public:
        static const char *BRDFLUT_VERTSHADERFILE;
        static const char *BRDFLUT_FRAGSHADERFILE;

        static const char *IBLFILTERING_VERTSHADERFILE;
        static const char *IBLFILTERING_FRAGSHADERFILE;

        static const int   ENVMAP_FILTERINGMIPSCOUNT;

};

}

#endif // PBRTOOLBOX_H
