#ifndef CENGINE_STORAGE_H_
#define CENGINE_STORAGE_H_

#include <vector>
#include "node.h"

enum class NodeStorageState {
    CREATED,
    INITIALIZED
};

class NodeStorage {
    public:
        std::vector<std::unique_ptr<Node>> nodes;
        std::vector<Node*> flatNodes;
        std::vector<Node*> newNodes;
        uint64_t nextId;
        NodeStorageState state = NodeStorageState::CREATED;

        NodeStorage(
            uint64_t nextId = 0
        ) {
            this->nextId = nextId;
        }

        void Init() {
            this->state = NodeStorageState::INITIALIZED;
        }

        uint64_t GetNextId() {
            return ++this->nextId;
        }

        void OnNestedNodeCreated(Node* newNode) {
            this->flatNodes.push_back(newNode);
            if (this->state == NodeStorageState::INITIALIZED) {
                this->newNodes.push_back(newNode);
            }
        }

        void InitNewNodes(cen::GameContext* ctx) {
            for (auto i = 0; i < this->newNodes.size(); i++) {
                this->newNodes[i]->Init(ctx);
            }
 
            this->newNodes.clear();
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
            if (this->state == NodeStorageState::INITIALIZED) {
                this->newNodes.push_back(nPtr);
            }
            return nPtr;
        }

        void RemoveFromIndex(Node* node) {
            for (auto i = 0; i < this->flatNodes.size(); i++) {
                if (this->flatNodes[i] == node) {
                    this->flatNodes.erase(this->flatNodes.begin() + i);
                    break;
                }
            }

            for (auto i = 0; i < this->newNodes.size(); i++) {
                if (this->newNodes[i] == node) {
                    this->newNodes.erase(this->newNodes.begin() + i);
                    break;
                }
            }
        }

        void RemoveFromIndexById(node_id_t id) {
            for (auto i = 0; i < this->flatNodes.size(); i++) {
                if (this->flatNodes[i]->id == id) {
                    this->flatNodes.erase(this->flatNodes.begin() + i);
                    break;
                }
            }

            for (auto i = 0; i < this->newNodes.size(); i++) {
                if (this->newNodes[i]->id == id) {
                    this->newNodes.erase(this->newNodes.begin() + i);
                    break;
                }
            }
        }

        void RemoveNode(Node* node) {
            this->RemoveFromIndex(node);

            for (auto i = 0; i < this->nodes.size(); i++) {
                if (this->nodes[i].get() == node) {
                    this->nodes.erase(this->nodes.begin() + i);
                    break;
                }
            }
        }

        void RemoveNodeById(node_id_t id) {
            this->RemoveFromIndexById(id);

            for (auto i = 0; i < this->nodes.size(); i++) {
                if (this->nodes[i]->id == id) {
                    this->nodes.erase(this->nodes.begin() + i);
                    break;
                }
            }
        }

        Node* GetById(node_id_t targetId) {
            for (const auto& node: this->nodes) {
                auto result = node->GetById(targetId);
                if (result != nullptr) {
                    return result;
                }
            }

            return nullptr;
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