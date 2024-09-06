#ifndef CENGINE_NODE_H_
#define CENGINE_NODE_H_

#include <vector>
#include <iostream>
#include "core.h"
#include "game_context.h"

// # Renderer and Updater

class Renderer {
    public:
        virtual void Render(GameContext* ctx) {};
};

class Updater {
    public:
        virtual void Update(GameContext* ctx) {};
};

class Initer {
    public:
        virtual void Init(GameContext* ctx) {};
};

// # Node Id Manager

typedef uint64_t node_id_t;

class NodeIdGenerator {
public:
    NodeIdGenerator(const NodeIdGenerator&) = delete;
    NodeIdGenerator& operator=(const NodeIdGenerator&) = delete;

    static NodeIdGenerator& GetInstance() {
        static NodeIdGenerator instance;
        return instance;
    }

    // Method to get the next ID
    node_id_t GetNextId() {
        std::lock_guard<std::mutex> lock(mutex_);
        return ++counter_;
    }

    node_id_t typeZero() {
        return 0;
    }

private:
    NodeIdGenerator() : counter_(0) {}

    node_id_t counter_;
    std::mutex mutex_;
};

// # Node

class NodeStorage;

class Node: public WithType, public Renderer, public Updater, public Initer {
    public:
        NodeStorage* storage;
        Node* parent;
        std::vector<std::unique_ptr<Node>> children;
        node_id_t id;
        bool activated = true;

        static const uint64_t _tid;

        type_id_t TypeId() const override {
            return Node::_tid;
        }

        Node(
            node_id_t id = 0,
            Node* parent = nullptr
        ) {
            this->parent = parent;
        }

        void Deactivate() {
            this->activated = false;
        }

        void Activate() {
            this->activated = true;
        }

        // # implementations in node_node_storage.h
        template <typename T>
        T* AddNode(std::unique_ptr<T> node);

        // # implementations in node_node_storage.h
        void RemoveChild(Node* node);
        
        // # implementations in node_node_storage.h
        void RemoveChildById(node_id_t id);

        Node* GetById(node_id_t targetId) {
            if (this->id == targetId) {
                return this;
            }

            for (const auto& node: this->children) {
                auto nestedFound = node->GetById(targetId);
                if (nestedFound != nullptr) {
                    return nestedFound;
                }
            }

            return nullptr;
        }

        template <typename T>
        T* GetById(node_id_t targetId) {
            static_assert(std::is_base_of<Node, T>::value, "T must inherit from Node");

            if (auto nPtr = dynamic_cast<T*>(this)) {
                if (nPtr->id == targetId) {
                    return nPtr;
                }
            }

            for (const auto& node: this->children) {
                auto nestedFound = node->GetById<T>(targetId);
                if (nestedFound != nullptr) {
                    return nestedFound;
                }
            }

            return nullptr;
        }

        template <typename T>
        void GetChildByType(std::vector<T*>& nodes) {
            for (const auto& node: this->children) {
                if (T* targetType = dynamic_cast<T*>(node.get())) {
                    nodes.push_back(targetType);
                }
            }
        }

        template <typename T>
        void GetChildByTypeDeep(std::vector<T*>& targetNodes) {
            for (const auto& childNode: this->children) {
                if (T* targetType = dynamic_cast<T*>(childNode.get())) {
                    targetNodes.push_back(targetType);
                }
                childNode->GetChildByTypeDeep<T>(targetNodes);
            }
        }

        template <typename T>
        T* GetFirstChildByType() {
            for (const auto& node: this->children) {
                if (auto targetNode = dynamic_cast<T*>(node.get())) {
                    return targetNode;
                }
            }

            return nullptr;
        }

        template <typename T>
        T* GetFirstByType() {
            if (auto targetNode = dynamic_cast<T*>(this)) {
                return targetNode;
            }

            return this->GetFirstChildByType<T>();
        }

        Node* RootNode() {
            if (this->parent != nullptr) {
                return this->parent->RootNode();
            }

            return this;
        };

        void TraverseInit(GameContext* ctx) {
            if (this->activated == false) {
                return;
            }

            this->Init(ctx);
            for (const auto& node: this->children) {
                node->TraverseInit(ctx);
            }
        }

        void TraverseUpdate(GameContext* ctx) {
            if (this->activated == false) {
                return;
            }

            this->Update(ctx);
            for (const auto& node: this->children) {
                node->TraverseUpdate(ctx);
            }
        };

        void TraverseRender(GameContext* ctx) {
            if (this->activated == false) {
                return;
            }

            this->Render(ctx);
            for (const auto& node: this->children) {
                node->TraverseRender(ctx);
            }
        }
};

#endif // CENGINE_NODE_H_