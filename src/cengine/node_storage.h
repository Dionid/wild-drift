#ifndef CENGINE_STORAGE_H_
#define CENGINE_STORAGE_H_

#include <vector>
#include "node.h"

class NodeStorage {
    public:
        std::vector<std::unique_ptr<Node>> nodes;
        std::vector<Node*> flatNodes;
        uint64_t nextId;

        NodeStorage(
            uint64_t nextId = 0
        ) {
            this->nextId = nextId;
        }

        uint64_t GetNextId() {
            return ++this->nextId;
        }

        void OnNestedNodeCreated(Node* newNode) {
            this->flatNodes.push_back(newNode);
        }

        template <typename T>
        T* AddNode(std::unique_ptr<T> node) {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");
            node->storage = this;
            T* nPtr = node.get();
            if (nPtr->id == 0) {
                nPtr->id = NodeIdGenerator::GetInstance().GetNextId();
            }
            this->nodes.push_back(std::move(node));
            this->flatNodes.push_back(nPtr);
            return nPtr;
        }

        template <typename T>
        T* GetById(node_id_t targetId) {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");

            for (const auto& node: this->nodes) {
                auto result = node->GetById<T>(targetId);
                if (result != nullptr) {
                    return result;
                }
            }

            return nullptr;
        }

        template <typename T>
        std::vector<T*> GetByType() {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");

            std::vector<Node*> nodes;

            for (const auto& node: this->nodes) {
                if (T* nPtr = dynamic_cast<T*>(node.get())) {
                    nodes.push_back(nPtr);
                }
            }

            return nodes;
        }

        template <typename T>
        T* GetFirstByType() {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");

            for (const auto& node: this->nodes) {
                if (T* nPtr = dynamic_cast<T*>(node.get())) {
                    return nPtr;
                }
            }

            return nullptr;
        }
};

#endif // CENGINE_STORAGE_H_