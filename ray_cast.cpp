 // Meant for simple inline inclusion

/*
struct raycast_result {
    bool hit;
    vector<double> location;
    vector<int> local_location;
    surface* surf;
};
*/

void raycast_from_camera_to_surface( surface* test, bool* dest_hit = NULL, vector<double>* dest_location = NULL, vector<int>* dest_local_location = NULL, surface** dest_surf = NULL ) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble modelMatrix[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

    GLdouble projMatrix[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

    GLdouble nearx, neary, nearz;
    GLdouble farx, fary, farz;

    gluUnProject(config_WIDTH / 2, config_HEIGHT / 2, 0.0, modelMatrix, projMatrix, viewport, &nearx, &neary, &nearz);
    gluUnProject(config_WIDTH / 2, config_HEIGHT / 2, 0.9, modelMatrix, projMatrix, viewport, &farx, &fary, &farz);

    vector<double> near      = vector<double>(nearx, neary, nearz);
    vector<double> far       = vector<double>(farx, fary, farz);
    vector<double> direction = far - near;
    vector<double> temp_vector;

    vector<double> nearest_result = vector<double>(0.0, 0.0, 0.0);
    bool nearest_valid = false;
    surface *nearest_surface = NULL;

    double temp, longest_distance = INFINITY;

    temp_vector = ray_cast( *test, near, direction );
    if (temp_vector != no_hit && (temp = ~(temp_vector - near) ) < longest_distance ) {
        nearest_valid = true;
        nearest_result = temp_vector;
        nearest_surface = test;
        longest_distance = temp;
    }

    if (dest_hit != NULL)
        *dest_hit = nearest_valid;
    if (dest_location != NULL)
        *dest_location = nearest_result;
    if (dest_local_location != NULL && nearest_valid) {
        matrix<double> localize_basis = matrix_recast<int, double>(&(nearest_surface->localize_basis));
        vector<int>local_location = vector_recast<double, int>( (localize_basis * nearest_result) + vector<double>(0.5, 0.0, 0.5) );
        local_location.z = nearest_surface->local_position.z;
        *dest_local_location = local_location;
    }
    if (dest_surf != NULL)
        *dest_surf = nearest_surface;

}

void raycast_from_camera( bool* dest_hit = NULL, vector<double>* dest_location = NULL, vector<int>* dest_local_location = NULL, surface** dest_surf = NULL ) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble modelMatrix[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

    GLdouble projMatrix[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

    GLdouble nearx, neary, nearz;
    GLdouble farx, fary, farz;

    gluUnProject(config_WIDTH / 2, config_HEIGHT / 2, 0.0, modelMatrix, projMatrix, viewport, &nearx, &neary, &nearz);
    gluUnProject(config_WIDTH / 2, config_HEIGHT / 2, 0.9, modelMatrix, projMatrix, viewport, &farx, &fary, &farz);

    vector<double> near      = vector<double>(nearx, neary, nearz);
    vector<double> far       = vector<double>(farx, fary, farz);
    vector<double> direction = far - near;
    vector<double> temp_vector;

    vector<double> nearest_result = vector<double>(0.0, 0.0, 0.0);
    bool nearest_valid = false;
    surface *nearest_surface = NULL;

    double temp, longest_distance = INFINITY;

    for (iter_surf=geometry.begin(); iter_surf != geometry.end(); ++iter_surf) {

        temp_vector = ray_cast( *iter_surf, near, direction );
        if (temp_vector != no_hit && (temp = ~(temp_vector - near) ) < longest_distance ) {
            nearest_valid = true;
            nearest_result = temp_vector;
            nearest_surface = &(*iter_surf);
            longest_distance = temp;
        }

    }

    if (dest_hit != NULL)
        *dest_hit = nearest_valid;
    if (dest_location != NULL)
        *dest_location = nearest_result;
    if (dest_local_location != NULL && nearest_valid) {
        matrix<double> localize_basis = matrix_recast<int, double>(&(nearest_surface->localize_basis));
        vector<double> global_float_location = localize_basis * nearest_result;
        if (global_float_location.x > 0) global_float_location.x += 0.5;
        else global_float_location.x -= 0.5;
        if (global_float_location.y > 0) global_float_location.y += 0.5;
        else global_float_location.y -= 0.5;

        vector<int>local_location = vector_recast<double, int>( global_float_location );
        local_location.z = nearest_surface->local_position.z;
        *dest_local_location = local_location;
    }
    if (dest_surf != NULL)
        *dest_surf = nearest_surface;

}

