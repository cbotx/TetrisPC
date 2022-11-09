#include "container_wrapper.h"
#include "definition.h"
#include "utils.h"

template <class T>
void ContainerWrapper<T>::loadHashSet(std::string filename) {
    container_read(*container, filename);
    container->insert(0); // TODO: add 0 in data file instead of here
#ifdef DEBUG_PRINT
    printf("%s size : %ull\n", filename.c_str(), hash_set->size());
#endif
}

template <class T>
void ContainerWrapper<T>::setHashSet(T* hs) {
    container = hs;
}