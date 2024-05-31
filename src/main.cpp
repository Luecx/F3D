#include "math/mat.h"
#include "math/transformation.h"

#include <iostream>
#include <memory>

void print_position(const std::string& name, const Vec3f& position) {
    std::cout << name << " Position: (" << position[0] << ", " << position[1] << ", " << position[2] << ")\n";
}

void test_transformations() {
    // Create transformations
    auto transformation1 = std::make_shared<Transformation>();
    auto transformation2 = std::make_shared<Transformation>();
    auto transformation3 = std::make_shared<Transformation>();

    // set positions to each
    transformation1->set_position({1.0f, 2.0f, 3.0f});
    transformation2->set_position({4.0f, 5.0f, 6.0f});
    transformation3->set_position({7.0f, 8.0f, 9.0f});

    // set chain of childs
    transformation1->add_child(transformation2);
    transformation2->add_child(transformation3);

    // print positions
    print_position("Transformation1", transformation1->global_position());
    print_position("Transformation2", transformation2->global_position());
    print_position("Transformation3", transformation3->global_position());
}

int main() {
    test_transformations();
    return 0;
}
