#ifndef GEOMETRY_RESOURCE_H
#define GEOMETRY_RESOURCE_H

#include "resource.h"

class GeometryResource : public Resource {
    public:
    GeometryResource(ResourceManager* manager, const std::string& name);

    protected:
    void do_load() override;
    void do_unload() override;
};

#endif // GEOMETRY_RESOURCE_H