#ifndef HITBOX_H
#define HITBOX_H

#include "PouEngine/utils/MathTools.h"

#include "tinyxml/tinyxml.h"

#include "Types.h"

#include <vector>

class Hitbox
{
    public:
        Hitbox();
        virtual ~Hitbox();

        bool loadFromXML(TiXmlElement *element);

        float getFactor(DamageType type) const;
        float getFactor(int type) const;
        const std::string &getSkeleton() const;
        const std::string &getNode() const;
        const pou::MathTools::Box &getBox() const;

    protected:

    private:
        std::string     m_skeleton;
        std::string     m_node;

        std::vector<float>  m_factors;
        pou::MathTools::Box m_box;
};

#endif // HITBOX_H