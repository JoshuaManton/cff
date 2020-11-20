#include "game.h"

bool ray_plane(Vector3 plane_normal, Vector3 plane_offset, Vector3 ray_origin, Vector3 ray_direction, Vector3 *out_hit_position) {
    float denom = dot(plane_normal, ray_direction);
    if (denom != 0) {
        Vector3 offset_to_plane = plane_offset - ray_origin;
        float t = dot(offset_to_plane, plane_normal) / denom;
        if (t >= 0) {
            *out_hit_position = ray_origin + ray_direction * t;
            return true;
        }
    }
    return false;
}

Vector3 unit_to_viewport(Vector3 a) {
    Vector3 result = (a * 2) - Vector3{1, 1, 0};
    return result;
}

Vector3 get_mouse_world_position(Matrix4 view, Matrix4 proj, Vector2 cursor_unit_position) {
    Vector4 cursor_viewport_position = v4(unit_to_viewport(v3(cursor_unit_position)));
    cursor_viewport_position.w = 1;

    Matrix4 inv = inverse(proj * view);

    Vector4 cursor_world_position4 = inv * cursor_viewport_position;
    if (cursor_world_position4.w != 0) {
        cursor_world_position4 /= cursor_world_position4.w;
    }
    return v3(cursor_world_position4);
}

// todo(josh): there should be a way to use the view matrix instead of having the camera_position parameter
Vector3 get_mouse_direction_from_camera(Vector3 camera_position, Matrix4 view, Matrix4 proj, Vector2 cursor_unit_position) {
    Vector3 cursor_world_position = get_mouse_world_position(view, proj, cursor_unit_position);
    Vector3 cursor_direction = normalize(cursor_world_position - camera_position);
    return cursor_direction;
}

void init_game_state(Game_State *game_state) {
    game_state->camera.orientation = quaternion_identity();
    game_state->camera.fov = 60;

    init_pool_allocator(&game_state->entities_pool, default_allocator(), sizeof(Entity), 1024); // todo(josh): @Leak
    game_state->active_entities = make_array<Entity *>(default_allocator(), 1024); // todo(josh): @Leak

    Entity *ship1 = make_entity(game_state, ENTITY_SHIP);
    ship1->orientation = quaternion_identity();
    ship1->ship.target_orientation = quaternion_identity();
    ship1->ship.top_speed = 1;
    ship1->ship.click_collision_radius = 1;
    ship1->ship.player_controlled = true;

    Entity *ship2 = make_entity(game_state, ENTITY_SHIP);
    ship2->orientation = quaternion_identity();
    ship2->ship.target_orientation = quaternion_identity();
    ship2->ship.top_speed = 1;
    ship2->ship.click_collision_radius = 1;
    ship2->ship.player_controlled = true;

    Entity *ship3 = make_entity(game_state, ENTITY_SHIP);
    ship3->orientation = quaternion_identity();
    ship3->ship.target_orientation = quaternion_identity();
    ship3->ship.top_speed = 1;
    ship3->ship.click_collision_radius = 1;
}

void update_game(Game_State *game_state, float dt, Window *window) {
    for (int i = game_state->active_entities.count-1; i >= 0; i--) {
        if (game_state->active_entities[i]->destroyed) {
            game_state->active_entities.unordered_remove(i);
        }
    }

    if (get_input_down(window, INPUT_F1)) {
        game_state->freecam = !game_state->freecam;
    }

    if (game_state->freecam) {
        const float CAMERA_SPEED_BASE = 5;
        const float CAMERA_SPEED_FAST = 20;
        const float CAMERA_SPEED_SLOW = 0.5;

        float camera_speed = CAMERA_SPEED_BASE;
             if (get_input(window, INPUT_SHIFT)) camera_speed = CAMERA_SPEED_FAST;
        else if (get_input(window, INPUT_ALT))   camera_speed = CAMERA_SPEED_SLOW;

        if (get_input(window, INPUT_E)) game_state->camera.position += quaternion_up(game_state->camera.orientation)      * camera_speed * dt;
        if (get_input(window, INPUT_Q)) game_state->camera.position -= quaternion_up(game_state->camera.orientation)      * camera_speed * dt;
        if (get_input(window, INPUT_W)) game_state->camera.position += quaternion_forward(game_state->camera.orientation) * camera_speed * dt;
        if (get_input(window, INPUT_S)) game_state->camera.position -= quaternion_forward(game_state->camera.orientation) * camera_speed * dt;
        if (get_input(window, INPUT_D)) game_state->camera.position += quaternion_right(game_state->camera.orientation)   * camera_speed * dt;
        if (get_input(window, INPUT_A)) game_state->camera.position -= quaternion_right(game_state->camera.orientation)   * camera_speed * dt;

        if (get_input(window, INPUT_MOUSE_RIGHT)) {
            Vector2 delta = window->mouse_position_pixel_delta * 0.25f;
            Vector3 rotate_vector = v3(-delta.y, delta.x, 0);

            Quaternion x = axis_angle(v3(1, 0, 0), to_radians(rotate_vector.x));
            Quaternion y = axis_angle(v3(0, 1, 0), to_radians(rotate_vector.y));
            Quaternion z = axis_angle(v3(0, 0, 1), to_radians(rotate_vector.z));
            Quaternion result = y * game_state->camera.orientation;
            result = result * x;
            result = result * z;
            result = normalize(result);
            game_state->camera.orientation = result;
        }
    }
    else {
        game_state->camera.position.y = 25;
        const float GAME_CAMERA_SPEED = 10;
        if (get_input(window, INPUT_W)) game_state->camera.position.z += GAME_CAMERA_SPEED * dt;
        if (get_input(window, INPUT_A)) game_state->camera.position.x -= GAME_CAMERA_SPEED * dt;
        if (get_input(window, INPUT_S)) game_state->camera.position.z -= GAME_CAMERA_SPEED * dt;
        if (get_input(window, INPUT_D)) game_state->camera.position.x += GAME_CAMERA_SPEED * dt;

        game_state->camera.orientation = quaternion_look_at(game_state->camera.position, v3(game_state->camera.position.x, 0, game_state->camera.position.z + 2), v3(0, 1, 0));
    }

    Vector3 mouse_dir = get_mouse_direction_from_camera(game_state->camera.position, camera_view_matrix(game_state->camera), camera_projection_matrix(game_state->camera, window), window->mouse_position_unit);

    if (!game_state->freecam) {
        Vector3 mouse_plane_pos = {};
        if (ray_plane(v3(0, 1, 0), v3(0, 0, 0), game_state->camera.position, mouse_dir, &mouse_plane_pos)) {
            if (get_input_down(window, INPUT_MOUSE_RIGHT)) {
                Entity *selected_ship = get_entity(game_state, game_state->selected_ship);
                if (selected_ship) {
                    ASSERT(selected_ship->kind == ENTITY_SHIP);
                    if (!get_input(window, INPUT_SHIFT)) {
                        selected_ship->ship.target_position = mouse_plane_pos;
                    }
                    selected_ship->ship.target_orientation = quaternion_look_at(selected_ship->position, mouse_plane_pos, v3(0, 1, 0));
                }
            }
            else if (get_input_down(window, INPUT_MOUSE_LEFT)) {
                Entity *closest = nullptr;
                float closest_distance = FLT_MAX;
                For (idx, game_state->active_entities) {
                    Entity *entity = game_state->active_entities[idx];
                    if (!entity->kind == ENTITY_SHIP) continue;
                    if (!entity->ship.player_controlled) continue;

                    float distance = length(mouse_plane_pos - entity->position);
                    if (distance < entity->ship.click_collision_radius && distance < closest_distance) {
                        closest_distance = distance;
                        closest = entity;
                    }
                }
                if (closest) {
                    game_state->selected_ship = closest->id;
                }
            }
        }
    }

    For (idx, game_state->active_entities) {
        Entity *entity = game_state->active_entities[idx];
        switch (entity->kind) {
            case ENTITY_SHIP: {
                if (length(entity->position - entity->ship.target_position) > 0.1) {
                    Vector3 dir_to_target = normalize(entity->ship.target_position - entity->position);
                    entity->position += dir_to_target * 5 * dt;
                }
                Quaternion diff = quaternion_difference(entity->orientation, entity->ship.target_orientation);
                entity->orientation = slerp(entity->orientation, diff * entity->orientation, 10 * dt);
                entity->orientation = normalize(entity->orientation);

                entity->ship.cur_shot_cooldown += dt;
                // todo(josh): framerate independence
                if (entity->ship.cur_shot_cooldown > 1) {
                    entity->ship.cur_shot_cooldown -= 1;
                    Entity *projectile = make_entity(game_state, ENTITY_PROJECTILE);
                    projectile->position = entity->position;
                    projectile->projectile.velocity = quaternion_forward(entity->orientation) * 50;
                }
                break;
            }
            case ENTITY_PROJECTILE: {
                entity->position += entity->projectile.velocity * dt;
                break;
            }
        }
    }
}



Entity *make_entity(Game_State *game_state, Entity_Kind kind) {
    int generation;
    int index;
    Entity *entity = (Entity *)pool_get(&game_state->entities_pool, &generation, &index);
    entity->id = encode_entity_id((u32)generation, (u32)index);
    entity->enabled = true;
    entity->kind = kind;
    entity->orientation = quaternion_identity();
    game_state->active_entities.append(entity);
    return entity;
}

void destroy_entity(Entity *entity) {
    entity->destroyed = true;
}

Entity *get_entity(Game_State *game_state, EntityID id) {
    u32 generation;
    u32 index;
    decode_entity_id(id, &generation, &index);
    Entity *entity = (Entity *)pool_get_slot_by_index(&game_state->entities_pool, index);
    if (entity->id == id && !entity->destroyed) {
        return entity;
    }
    return nullptr;
}

EntityID encode_entity_id(u32 generation, u32 index) {
    return (EntityID)(((u64)generation << 32) | ((u64)index));
}

void decode_entity_id(EntityID id, u32 *out_generation, u32 *out_index) {
    *out_generation = (u32)(((u64)id) >> 32);
    *out_index = (u32)id;
}



Matrix4 camera_view_matrix(Camera camera) {
    return construct_view_matrix(camera.position, camera.orientation);
}

Matrix4 camera_projection_matrix(Camera camera, Window *window) {
    return construct_perspective_matrix(to_radians(camera.fov), (float)window->width / (float)window->height, 0.001, 1000);
}