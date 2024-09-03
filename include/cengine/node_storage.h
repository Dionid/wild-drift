#ifndef CENGINE_STORAGE_H_
#define CENGINE_STORAGE_H_

#include <vector>
#include "node.h"

class NodeStorage {
    public:
        std::vector<std::unique_ptr<Node>> nodes;

        template <typename T>
        T* AddNode(std::unique_ptr<T> node) {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");
            T* nPtr = node.get();
            this->nodes.push_back(std::move(node));
            return nPtr;
        }

        template <typename T>
        T* GetByPtr(T* targetPtr) {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");

            for (const auto& node: this->nodes) {
                auto nPtr = node.get();
                if (targetPtr == nPtr) {
                    return static_cast<T*>(nPtr);
                }
            }

            return nullptr;
        }

        template <typename T>
        std::vector<T*> GetByType() {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");

            std::vector<Node*> nodes;

            for (const auto& node: this->nodes) {
                auto nPtr = node.get();
                if (T::_id == nPtr->TypeId()) {
                    nodes.push_back(
                        static_cast<T*>(nPtr)
                    );
                }
            }

            return nodes;
        }

        template <typename T>
        T* GetFirstByType() {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");

            for (const auto& node: this->nodes) {
                auto nPtr = node.get();
                if (T::_id == nPtr->TypeId()) {
                    return static_cast<T*>(nPtr);
                }
            }

            return nullptr;
        }
};

#endif // CENGINE_STORAGE_H_