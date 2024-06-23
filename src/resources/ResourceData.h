#ifndef RESOURCEDATA_H
#define RESOURCEDATA_H

#include <ecs.h>
#include <memory>
#include <string>

enum Type { LOAD, UNLOAD };
enum State { CPU, GPU };

class ResourceManager;

struct ResourceData : public std::enable_shared_from_this<ResourceData> {
    private:
    std::string path;
    ecs::ID id;
    ResourceManager* manager = nullptr;

    bool loaded_cpu = false;
    bool loaded_gpu = false;

    public:
    ResourceData(const std::string& path);

    void set_manager(ResourceManager* manager);

    const std::string& get_path() const { return path; } // Added getter for path

    protected:
    virtual bool load_cpu_specific() = 0;
    virtual bool load_gpu_specific() = 0;
    virtual bool unload_cpu_specific() = 0;
    virtual bool unload_gpu_specific() = 0;

    bool load_cpu();
    bool load_gpu();
    bool unload_cpu();
    bool unload_gpu();

    public:
    bool operator()(State state, Type type);

    void load(State state);
    void unload(State state);
};

#endif // RESOURCEDATA_H
