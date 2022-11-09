#pragma once

#include <string>

template <class T>
class ContainerWrapper {
   private:
    T* container;

   public:
    ContainerWrapper() = default;

    void loadHashSet(std::string filename);

    void setHashSet(T* hs);
};