#ifndef IMAGE_RESOURCE_H
#define IMAGE_RESOURCE_H

#include "resource.h"

class ImageResource : public Resource {
    public:
    ImageResource(ResourceManager* manager, const std::string& name);

    protected:
    void do_load() override;
    void do_unload() override;
};

#endif // IMAGE_RESOURCE_H