#pragma once

#include "window.h"
#include "math.h"

typedef u64 EntityID;

struct Camera {
    Vector3 position;
    Quaternion orientation;
    float fov;
};

struct Weapon {
    Vector3 offset_from_ship_position;
    Vector3 facing_direction;
    float effective_angle;
    float cur_shot_cooldown;
    float shot_cooldown;
    Vector4 missile_color;
    EntityID current_target_id;
};

struct Ship {
    Vector3 target_position;
    Vector3 velocity;
    float top_speed;
    Quaternion target_orientation;
    float click_collision_radius;
    bool player_controlled;
    Weapon weapons[8];
    int num_weapons;
};

struct Projectile {
    float time_to_live;
};

enum Entity_Kind {
    ENTITY_INVALID,
    ENTITY_SHIP,
    ENTITY_PROJECTILE,
};

struct Entity {
    EntityID id;
    bool enabled;
    bool destroyed;
    Vector3 position;
    Quaternion orientation;
    Vector3 velocity;
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
void destroy_entity(Entity *entity);
Entity *get_entity(Game_State *game_state, EntityID id);
EntityID encode_entity_id(u32 generation, u32 index);
void decode_entity_id(EntityID id, u32 *out_generation, u32 *out_index);

Vector3 get_weapon_position(Weapon *weapon, Entity *ship);
Vector3 get_weapon_direction(Weapon *weapon, Entity *ship);

Matrix4 camera_view_matrix(Camera camera);
Matrix4 camera_projection_matrix(Camera camera, Window *window);