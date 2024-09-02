#ifndef CENGINE_STORAGE_H_
#define CENGINE_STORAGE_H_

#include <vector>
#include "node.h"

class NodeStorage {
    public:
        std::vector<std::unique_ptr<Node>> nodes;

        void AddNode(std::unique_ptr<Node> node) {
            this->nodes.push_back(std::move(node));
        }

        template <typename T>
        std::vector<Node*> GetByType() {
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