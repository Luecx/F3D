#ifndef RESOURCE_H
#define RESOURCE_H

#include "../gldata/GLData.h"

#include <atomic>
#include <string>

class ResourceManager;

class Resource : public std::enable_shared_from_this<Resource>{

    public:
    GLData data_id{};
    ResourceManager* resource_manager;
    int ref_count = 0;

    public:
    Resource(ResourceManager* manager, const std::string& name);
    virtual ~Resource() = default;

    void require();
    void unrequire();

    void load();
    void unload();

    bool is_loaded() const { return loaded_; }
    const std::string& get_name() const { return name_; }

    protected:
    virtual void do_load() = 0;
    virtual void do_unload() = 0;

    protected:
    std::string name_;
    std::atomic<bool> loaded_;

    friend class ResourceManager;
};

#endif // RESOURCE_H