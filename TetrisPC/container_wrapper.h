#pragma once

#include <string>

template <class T, class V>
class ContainerWrapper {
   private:
    T* container = nullptr;

   public:
    ContainerWrapper() = default;

    void loadHashSet(std::string filename);

    void setHashSet(T* hs);

    void insert(V&& item);

    bool contains(V&& item);
};