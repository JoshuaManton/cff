#pragma once

#include "window.h"
#include "math.h"
#include "renderer.h"

typedef u64 EntityID;

struct Ship_Models {
    Array<Loaded_Mesh> small_ship_meshes;
    Array<Loaded_Mesh> big_ship_meshes;
    Array<Loaded_Mesh> sniper_ship_meshes;
};

struct Camera {
    Vector3 position;
    Quaternion orientation;
    float fov;
};

struct Move_Command {
    Vector3 to;
};

struct Rotate_Command {
    Vector3 position_to_rotate_towards;
};

enum Unit_Command_Kind {
    UNIT_MOVE_COMMAND,
    UNIT_ROTATE_COMMAND,

    UNIT_COMMAND_COUNT,
};

struct Unit_Command {
    Unit_Command_Kind kind;
    union {
        Move_Command move;
        Rotate_Command rotate;
    };
};

struct Weapon {
    Vector3 offset_from_ship_position;
    Vector3 facing_direction;
    float range;
    float effective_angle;
    float cur_shot_cooldown;
    float shot_cooldown;
    Vector4 projectile_color;
    EntityID current_target_id;
};

struct Ship_Definition {
    Array<Loaded_Mesh> *model;
    float move_speed;
    Weapon weapons[8];
    int num_weapons;
};

struct Ship {
    Vector3 target_position;
    Vector3 velocity;
    float top_speed;
    Quaternion target_orientation;
    float collision_radius;
    bool player_controlled;
    Ship_Definition definition;
    Unit_Command commands[16];
    int command_cursor;
    int num_commands;
};

struct Projectile {
    EntityID shooter_id;
    float time_to_live;
    // todo(josh): model
    Vector4 color;
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

    bool gameplay_paused;
    EntityID selected_ship;

    bool freecam;

    Pool_Allocator entities_pool;
    Array<Entity *> active_entities;
};

void init_game_state(Game_State *game_state, Ship_Models *ship_models);
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