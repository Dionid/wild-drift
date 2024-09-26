#ifndef CENGINE_STORAGE_H_
#define CENGINE_STORAGE_H_

#include <vector>
#include "node.h"
#include "node_2d.h"

namespace cen {

enum class NodeStorageState {
    CREATED,
    INITIALIZED
};

class Scene;

class NodeStorage {
    public:
        Scene* scene;
        std::vector<std::unique_ptr<Node>> rootNodes;
        std::vector<Node*> flatNodes;
        std::vector<Node*> newNodes;
        std::vector<Node2D*> renderNodes;
        uint64_t nextId;
        NodeStorageState state = NodeStorageState::CREATED;

        NodeStorage(
            Scene* scene = nullptr,
            uint64_t nextId = 0
        ) {
            this->scene = scene;
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
            if (Node2D* n2d = dynamic_cast<Node2D*>(newNode)) {
                this->renderNodes.push_back(n2d);
            }
        }

        void InitNewNodes() {
            for (auto i = 0; i < this->newNodes.size(); i++) {
                this->newNodes[i]->Init();
            }
 
            this->newNodes.clear();
        }

        template <typename T>
        T* AddNode(std::unique_ptr<T> newNode) {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");
            newNode->storage = this;
            newNode->scene = this->scene;
            T* nPtr = newNode.get();
            if (nPtr->id == 0) {
                nPtr->id = NodeIdGenerator::GetInstance().GetNextId();
            }
            this->rootNodes.push_back(std::move(newNode));
            this->flatNodes.push_back(nPtr);
            if (this->state == NodeStorageState::INITIALIZED) {
                this->newNodes.push_back(nPtr);
            }
            if (Node2D* n2d = dynamic_cast<Node2D*>(nPtr)) {
                this->renderNodes.push_back(n2d);
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

            for (auto i = 0; i < this->rootNodes.size(); i++) {
                if (this->rootNodes[i].get() == node) {
                    this->rootNodes.erase(this->rootNodes.begin() + i);
                    break;
                }
            }
        }

        void RemoveNodeById(node_id_t id) {
            this->RemoveFromIndexById(id);

            for (auto i = 0; i < this->rootNodes.size(); i++) {
                if (this->rootNodes[i]->id == id) {
                    this->rootNodes.erase(this->rootNodes.begin() + i);
                    break;
                }
            }
        }

        Node* GetById(node_id_t targetId) {
            for (const auto& node: this->rootNodes) {
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

            for (const auto& node: this->rootNodes) {
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

            for (const auto& node: this->rootNodes) {
                if (T* nPtr = dynamic_cast<T*>(node.get())) {
                    nodes.push_back(nPtr);
                }
            }

            return nodes;
        }

        template <typename T>
        T* GetFirstByType() {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");

            for (const auto& node: this->rootNodes) {
                if (T* nPtr = dynamic_cast<T*>(node.get())) {
                    return nPtr;
                }
            }

            return nullptr;
        }
};

} // namespace cen

#endif // CENGINE_STORAGE_H_