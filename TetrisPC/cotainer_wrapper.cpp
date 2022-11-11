#include "container_wrapper.h"
#include "definition.h"
#include "utils.h"

template <class T, class V>
void ContainerWrapper<T, V>::loadHashSet(std::string filename) {
    container_read(*container, filename);
    container->insert(0);  // TODO: add 0 in data file instead of here
#ifdef DEBUG_PRINT
    printf("%s size : %ull\n", filename.c_str(), hash_set->size());
#endif
}

template <class T, class V>
void ContainerWrapper<T, V>::setHashSet(T* hs) {
    container = hs;
}

template <class T, class V>
void ContainerWrapper<T, V>::insert(V&& item) {
    container->insert(std::forward<V>(item));
}

template <class T, class V>
bool ContainerWrapper<T, V>::contains(V&& item) {
    return container->contains(item);
}