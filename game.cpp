#include "game.h"

/*
TODO:
-crew members
-ship stats
-ship health
-weapon stats
-ship definitions
-different kinds of weapon slots (lasers, turrets, etc)
-ship abilities
*/

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

    Ship_Definition small_ship = {};
    small_ship.weapons[0].facing_direction = v3(0, 0, 1);
    small_ship.weapons[0].effective_angle = 10;
    small_ship.weapons[0].shot_cooldown = 0.5;
    small_ship.weapons[0].projectile_color = v4(100, 20, 20, 1);
    small_ship.weapons[0].offset_from_ship_position = v3(-1, 0, 0);
    small_ship.weapons[0].range = 10;
    small_ship.weapons[1].facing_direction = v3(0, 0, 1);
    small_ship.weapons[1].effective_angle = 10;
    small_ship.weapons[1].shot_cooldown = 0.5;
    small_ship.weapons[1].projectile_color = v4(100, 20, 20, 1);
    small_ship.weapons[1].offset_from_ship_position = v3(1, 0, 0);
    small_ship.weapons[1].range = 10;
    small_ship.num_weapons = 2;

    Ship_Definition big_ship = {};
    big_ship.weapons[0].facing_direction = v3(1, 0, 0);
    big_ship.weapons[0].effective_angle = 30;
    big_ship.weapons[0].shot_cooldown = 1;
    big_ship.weapons[0].projectile_color = v4(100, 20, 20, 1);
    big_ship.weapons[0].offset_from_ship_position = v3(1, 0, 1);
    big_ship.weapons[0].range = 15;
    big_ship.weapons[1].facing_direction = v3(1, 0, 0);
    big_ship.weapons[1].effective_angle = 30;
    big_ship.weapons[1].shot_cooldown = 1;
    big_ship.weapons[1].projectile_color = v4(100, 20, 20, 1);
    big_ship.weapons[1].offset_from_ship_position = v3(1, 0, -1);
    big_ship.weapons[1].range = 15;
    big_ship.weapons[2].facing_direction = v3(-1, 0, 0);
    big_ship.weapons[2].effective_angle = 30;
    big_ship.weapons[2].shot_cooldown = 1;
    big_ship.weapons[2].projectile_color = v4(100, 20, 20, 1);
    big_ship.weapons[2].offset_from_ship_position = v3(-1, 0, 1);
    big_ship.weapons[2].range = 15;
    big_ship.weapons[3].facing_direction = v3(-1, 0, 0);
    big_ship.weapons[3].effective_angle = 30;
    big_ship.weapons[3].shot_cooldown = 1;
    big_ship.weapons[3].projectile_color = v4(100, 20, 20, 1);
    big_ship.weapons[3].offset_from_ship_position = v3(-1, 0, -1);
    big_ship.weapons[3].range = 15;
    big_ship.num_weapons = 4;

    Entity *ship1 = make_entity(game_state, ENTITY_SHIP);
    ship1->orientation = quaternion_identity();
    ship1->ship.target_orientation = quaternion_identity();
    ship1->ship.top_speed = 1;
    ship1->ship.collision_radius = 1;
    ship1->ship.player_controlled = true;
    ship1->ship.definition = small_ship;

    Entity *ship2 = make_entity(game_state, ENTITY_SHIP);
    ship2->orientation = quaternion_identity();
    ship2->ship.target_orientation = quaternion_identity();
    ship2->ship.top_speed = 1;
    ship2->ship.collision_radius = 1;
    ship2->ship.player_controlled = true;
    ship2->ship.definition = big_ship;

    Entity *ship3 = make_entity(game_state, ENTITY_SHIP);
    ship3->orientation = quaternion_identity();
    ship3->ship.target_orientation = quaternion_identity();
    ship3->ship.top_speed = 1;
    ship3->ship.collision_radius = 1;
}

bool target_is_valid(Entity *target, Entity *shooter, Weapon *weapon) {
    if (target->kind != ENTITY_SHIP) return false;
    if (target == shooter) return false;
    Vector3 offset_to_ship = target->position - get_weapon_position(weapon, shooter);
    float distance_to_ship = length(offset_to_ship);
    if (distance_to_ship > weapon->range) {
        return false;
    }

    Vector3 dir_to_ship = offset_to_ship / distance_to_ship;
    Vector3 weapon_direction = get_weapon_direction(weapon, shooter);
    float angle_between = to_degrees(acos(dot(get_weapon_direction(weapon, shooter), dir_to_ship) / (length(weapon_direction) * length(dir_to_ship))));
    if (fabsf(angle_between) <= weapon->effective_angle) {
        return true;
    }
    return false;
}

Entity *find_weapon_target(Game_State *game_state, Weapon *weapon, Entity *parent_ship) {
    For (idx, game_state->active_entities) {
        Entity *entity = game_state->active_entities[idx];
        if (target_is_valid(entity, parent_ship, weapon)) {
            return entity;
        }
    }
    return nullptr;
}

Vector3 get_weapon_position(Weapon *weapon, Entity *ship) {
    return ship->position + ship->orientation * weapon->offset_from_ship_position;
}

Vector3 get_weapon_direction(Weapon *weapon, Entity *ship) {
    return normalize(ship->orientation * weapon->facing_direction);
}

void update_game(Game_State *game_state, float dt, Window *window) {
    for (int i = game_state->active_entities.count-1; i >= 0; i--) {
        if (game_state->active_entities[i]->destroyed) {
            pool_return(&game_state->entities_pool, game_state->active_entities[i]);
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
        if (get_input_down(window, INPUT_SPACE)) {
            game_state->gameplay_paused = !game_state->gameplay_paused;
        }

        Vector3 mouse_plane_pos = {};
        if (ray_plane(v3(0, 1, 0), v3(0, 0, 0), game_state->camera.position, mouse_dir, &mouse_plane_pos)) {
            mouse_plane_pos.y = 0;
            if (get_input_down(window, INPUT_MOUSE_RIGHT)) {
                Entity *selected_ship = get_entity(game_state, game_state->selected_ship);
                if (selected_ship && (selected_ship->ship.num_commands < 16)) {
                    ASSERT(selected_ship->kind == ENTITY_SHIP);
                    Unit_Command *next_unit_command = &selected_ship->ship.commands[(selected_ship->ship.command_cursor + selected_ship->ship.num_commands) % ARRAYSIZE(selected_ship->ship.commands)];

                    if (!get_input(window, INPUT_SHIFT)) {
                        selected_ship->ship.command_cursor = (selected_ship->ship.command_cursor + selected_ship->ship.num_commands) % ARRAYSIZE(selected_ship->ship.commands);
                        selected_ship->ship.num_commands = 0;
                    }

                    if (get_input(window, INPUT_CONTROL)) {
                        next_unit_command->kind = UNIT_ROTATE_COMMAND;
                        next_unit_command->rotate.position_to_rotate_towards = mouse_plane_pos;
                        selected_ship->ship.num_commands += 1;
                        // selected_ship->ship.target_orientation = quaternion_look_at(selected_ship->position, mouse_plane_pos, v3(0, 1, 0));
                    }
                    else {
                        next_unit_command->kind = UNIT_MOVE_COMMAND;
                        next_unit_command->move.to = mouse_plane_pos;
                        selected_ship->ship.num_commands += 1;
                    }
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
                    if (distance < entity->ship.collision_radius && distance < closest_distance) {
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

    if (!game_state->gameplay_paused) {
        For (idx, game_state->active_entities) {
            Entity *entity = game_state->active_entities[idx];
            entity->position += entity->velocity * dt;
            switch (entity->kind) {
                case ENTITY_SHIP: {
                    if (entity->ship.num_commands > 0) {
                        Unit_Command *command = &entity->ship.commands[entity->ship.command_cursor];
                        #define COMPLETE_COMMAND { entity->ship.num_commands -= 1; entity->ship.command_cursor = (entity->ship.command_cursor + 1) % ARRAYSIZE(entity->ship.commands); }
                        switch (command->kind) {
                            case UNIT_MOVE_COMMAND: {
                                if (length(entity->position - command->move.to) > 0.1) {
                                    Quaternion required_orientation = quaternion_look_at(entity->position, command->move.to, v3(0, 1, 0));
                                    if (to_degrees(angle_between_quaternions(entity->orientation, required_orientation)) > 1) {
                                        Quaternion diff = quaternion_difference(entity->orientation, required_orientation);
                                        entity->orientation = slerp(entity->orientation, normalize(diff * entity->orientation), 5 * dt);
                                        entity->orientation = normalize(entity->orientation);
                                    }

                                    if (to_degrees(angle_between_quaternions(entity->orientation, required_orientation)) < 30) {
                                        entity->position += quaternion_forward(entity->orientation) * 3 * dt;
                                    }
                                }
                                else {
                                    COMPLETE_COMMAND;
                                }
                                break;
                            }
                            case UNIT_ROTATE_COMMAND: {
                                Quaternion required_orientation = quaternion_look_at(entity->position, command->rotate.position_to_rotate_towards, v3(0, 1, 0));
                                if (to_degrees(angle_between_quaternions(entity->orientation, required_orientation)) > 1) {
                                    Quaternion diff = quaternion_difference(entity->orientation, required_orientation);
                                    entity->orientation = slerp(entity->orientation, normalize(diff * entity->orientation), 5 * dt);
                                    entity->orientation = normalize(entity->orientation);
                                }
                                else {
                                    COMPLETE_COMMAND;
                                }
                                break;
                            }
                        }
                    }

                    // if (to_degrees(angle_between_quaternions(entity->orientation, entity->ship.target_orientation)) > 0.01) {
                    //     Quaternion diff = quaternion_difference(entity->orientation, entity->ship.target_orientation);
                    //     entity->orientation = slerp(entity->orientation, diff * entity->orientation, 10 * dt);
                    //     entity->orientation = normalize(entity->orientation);
                    // }

                    for (int i = 0; i < entity->ship.definition.num_weapons; i++) {
                        Weapon *weapon = &entity->ship.definition.weapons[i];
                        if (weapon->cur_shot_cooldown >= 0) {
                            weapon->cur_shot_cooldown -= dt;
                        }

                        Entity *target = get_entity(game_state, weapon->current_target_id);
                        if (target) {
                            if (!target_is_valid(target, entity, weapon)) {
                                weapon->current_target_id = 0;
                                target = nullptr;
                            }
                        }

                        if (!target) {
                            target = find_weapon_target(game_state, weapon, entity);
                        }

                        if (target) {
                            weapon->current_target_id = target->id;

                            // todo(josh): framerate independence
                            if (weapon->cur_shot_cooldown <= 0) {
                                weapon->cur_shot_cooldown += weapon->shot_cooldown;
                                ASSERT(target->kind == ENTITY_SHIP);
                                Entity *projectile = make_entity(game_state, ENTITY_PROJECTILE);
                                Vector3 weapon_position = get_weapon_position(weapon, entity);
                                projectile->position = weapon_position;
                                projectile->velocity = normalize(target->position - weapon_position) * 20;
                                projectile->projectile.time_to_live = 5;
                                projectile->projectile.shooter_id = entity->id;
                                projectile->projectile.color = weapon->projectile_color;
                            }
                        }
                        else {
                            weapon->current_target_id = 0;
                        }
                    }
                    break;
                }
                case ENTITY_PROJECTILE: {
                    entity->projectile.time_to_live -= dt;
                    if (entity->projectile.time_to_live <= 0) {
                        destroy_entity(entity);
                    }
                    else {
                        Entity *shooter = get_entity(game_state, entity->projectile.shooter_id);
                        For (idx, game_state->active_entities) {
                            Entity *other = game_state->active_entities[idx];
                            if (other->kind != ENTITY_SHIP) continue;
                            if (shooter != nullptr && other == shooter) continue; // todo(josh): proper friendly fire detection

                            float distance = length(entity->position - other->position);
                            if (distance < other->ship.collision_radius) {
                                destroy_entity(entity);
                            }
                        }
                    }
                    break;
                }
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