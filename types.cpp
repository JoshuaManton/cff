struct Vertex {
    Vector3 position;
    Vector3 tex_coord;
    Vector4 color;
};

struct Texture_Description {
    int width;
    int height;
    bool render_target;
    // format
    // type
    // is_cpu_read_target
    byte *color_data;
};