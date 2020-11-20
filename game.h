#pragma once

#include "window.h"
#include "math.h"

struct Camera {
    Vector3 position;
    Quaternion orientation;
    float fov;
};

struct Ship {
    Vector3 target_position;
    Vector3 velocity;
    float top_speed;
    Quaternion target_orientation;
    float click_collision_radius;
    bool player_controlled;
    float cur_shot_cooldown;
};

struct Projectile {
    Vector3 velocity;
};

enum Entity_Kind {
    ENTITY_INVALID,
    ENTITY_SHIP,
    ENTITY_PROJECTILE,
};

typedef u64 EntityID;
struct Entity {
    EntityID id;
    bool enabled;
    bool destroyed;
    Vector3 position;
    Quaternion orientation;
    Entity_Kind kind;
    union {
        Ship ship;
        Projectile projectile;
    };
};

struct Game_State {
    Camera camera;

    EntityID selected_ship;

    bool freecam;

    Pool_Allocator entities_pool;
    Array<Entity *> active_entities;
};

void init_game_state(Game_State *game_state);
void update_game(Game_State *game_state, float dt, Window *window);

Entity *make_entity(Game_State *game_state, Entity_Kind kind = ENTITY_INVALID);
Entity *get_entity(Game_State *game_state, EntityID id);
EntityID encode_entity_id(u32 generation, u32 index);
void decode_entity_id(EntityID id, u32 *out_generation, u32 *out_index);

Matrix4 camera_view_matrix(Camera camera);
Matrix4 camera_projection_matrix(Camera camera, Window *window);