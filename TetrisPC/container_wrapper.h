#pragma once

#include <string>
#include <utility>

#include "container_wrapper.h"
#include "definition.h"
#include "utils.h"

template <class T, class V>
class ContainerWrapper {
   private:
    T* container = nullptr;
    bool original;

   public:
    ContainerWrapper();

    explicit ContainerWrapper(const ContainerWrapper<T, V>& wr);

    explicit ContainerWrapper(std::string filename);

    virtual ~ContainerWrapper();

    void WriteToFile(std::string filename);

    void insert(V&& item);

    bool contains(const V& item);
};

template <class T, class V>
ContainerWrapper<T, V>::ContainerWrapper() : original(true) {
    container = new T;
}

template <class T, class V>
ContainerWrapper<T, V>::ContainerWrapper(std::string filename) : original(true) {
    container = new T;
    container_read(*container, filename);
    container->insert(0);  // TODO: add 0 in data file instead of here
#ifdef DEBUG_PRINT
    printf("%s size : %ull\n", filename.c_str(), container->size());
#endif
}

template <class T, class V>
ContainerWrapper<T, V>::ContainerWrapper(const ContainerWrapper<T, V>& wr) : original(false) {
    container = wr.container;
}

template <class T, class V>
ContainerWrapper<T, V>::~ContainerWrapper() {
    if (original && container) delete container;
}

template <class T, class V>
void ContainerWrapper<T, V>::WriteToFile(std::string filename) {
    container_write(*container, filename);
}

template <class T, class V>
void ContainerWrapper<T, V>::insert(V&& item) {
    container->insert(std::forward<V>(item));
}

template <class T, class V>
bool ContainerWrapper<T, V>::contains(const V& item) {
    return container->contains(item);
}
