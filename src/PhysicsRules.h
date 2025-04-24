
#pragma once
#include "Definition.h"
#include "MathUtils.h"

float remap01(float inp, float inp_start, float inp_end) {
    return MathUtils::Clamp((inp - inp_start) / (inp_end - inp_start), 0.0, 1.0);
}
float dist_sqr(Vector3f a, Vector3f b) {
    Vector3f diff = a - b;
    return diff.dot(diff);
}

// ------------------------------------------------------------
// Particle structure
struct Particle {
    Vector3f pos;
    Vector3f pos_prev;
    Vector3f vel;
    float inv_mass;
    bool is_fixed;
};


// Simulation constants
const float damp = 0.4;
const float collision_dist = 0.2;
const float ground_collision_dist = 0.1;
const Vector3f gravity = Vector3f(0.0, -1, 0.0);

// Define n_rope rope particles and add one extra "mouse particle".
const int MAX_PARTICLES = 20;
const int MAX_SPRINGS = 20;

//0: mouse particle
//1...5: rope particles
int n_particles;
Particle particles[MAX_PARTICLES];

int nearest_particle(Vector3f p) {
    int idx = 1;
    float min_dist = 1e9;
    for (int i = 1; i < n_particles; i++) {
        float d = dist_sqr(p, particles[i].pos);
        if (d < min_dist) {
            min_dist = d;
            idx = i;
        }
    }
    return idx;
}

// ------------------------------------------------------------
// Spring structure
struct Spring {
    int a;
    int b;
    float restLength;
    float inv_stiffness;
};
// Create springs between adjacent rope particles (n_rope-1 springs)
// and one spring connecting the last rope particle and the mouse particle.
Spring springs[MAX_SPRINGS];
int n_springs;
int selected_particle = -1;
int current_add_particle = -1;

Spring add_spring(int a, int b, float inv_stiffness){
    Spring s;
    s.a = a;
    s.b = b;
    s.restLength = (particles[a].pos - particles[b].pos).norm();
    s.inv_stiffness = inv_stiffness;
    return s;
}

const int initial_particles = 6;

void init_state(void){
    n_particles = 6;
    n_springs = 5;

    //particle 0 is the mouse particle and will be set later
    particles[1].pos = Vector3f(-0.6, 0.5, 0.0); 
    particles[1].vel = Vector3f(0.0, 0.0, 0.0);
    particles[2].pos = Vector3f(-0.3, 0.5, 0.0); 
    particles[2].vel = Vector3f(0.0, 0.0, 0.0);
    particles[3].pos = Vector3f(-0, 0.5, 0.0);
    particles[3].vel = Vector3f(0.0, 0.0, 0.0);
    particles[4].pos = Vector3f(0.3, 0.5, 0.0);
    particles[4].vel = Vector3f(0.0, 0.0, 0.0);
    particles[5].pos = Vector3f(0.6, 0.5), 0.0;
    particles[5].vel = Vector3f(0.0, 0.0, 0.0);

    current_add_particle = initial_particles;

    // Springs between adjacent rope particles
    //spring 0 is the mouse particle to the first rope particle
    springs[1] = add_spring(1, 2, 1.0 / 100.0); // first to second rope particle
    springs[2] = add_spring(2, 3, 1.0 / 100.0); // second to third rope particle
    springs[3] = add_spring(3, 4, 1.0 / 100.0); // third to fourth rope particle
    springs[4] = add_spring(4, 5, 1.0 / 100.0); // fourth to fifth rope particle
}

/////////////////////////////////////////////////////
//// Step 1: Computing the spring constraint
//// This function calculates the deviation of a spring's length 
//// from its rest length. The constraint is defined as L - L0, 
//// This constraint is later used to adjust the positions of particles 
//// to enforce the spring constraint.
/////////////////////////////////////////////////////
float spring_constraint(Spring s) {
    // The spring has two endpoints a and b.
    // Their positions are particles[s.a].pos and particles[s.b].pos respectively.
    // The spring constraint is L-L0, where L is the current length of the spring
    // and L0 = s.restLength is the rest length of the spring.

    //// Your implementation starts
    return (particles[s.a].pos - particles[s.b].pos).norm() - s.restLength;
    //// Your implementation ends
}

/////////////////////////////////////////////////////
//// Step 2: Computing the spring constraint gradient
//// This function calculates the gradient of the spring constraint constraint 
//// for a spring a--b with respect to the position of a.
/////////////////////////////////////////////////////
Vector3f spring_constraint_gradient(Vector3f a, Vector3f b) {
    // Gradient of the spring constraint for points a,b with respect to a.
    // Think: what is the gradient of (a-b) with respect to a?

    //// Your implementation starts
    float denominator = (a-b).norm();
    if(denominator == 0.0){
        return Vector3f(0.0, 0.0, 0.0);
    }
    return (a-b)/denominator;
    //// Your implementation ends
}

// Compute the gradient of the spring constraint with respect to a given particle.
Vector3f spring_constraint_grad(Spring s, int particle_idx) {
    float sgn = (particle_idx == s.a) ? 1.0 : -1.0;
    return sgn * spring_constraint_gradient(particles[s.a].pos, particles[s.b].pos);
}

/////////////////////////////////////////////////////
//// Step 3: Solving a single spring constraint
//// Calculate the numerator and denominator for the Lagrangian multiplier update.
//// You will calculate the numer/denom for PBD updates.
//// The Lagrangian multiplier update is calculated with lambda=-(numer/denom)
//// See the documentation for more details.
/////////////////////////////////////////////////////
void solve_spring(Spring s, float dt) {   
    float numer = 0.;
    float denom = 0.;

    //// Your implementation starts
    Vector3f grad_a = spring_constraint_grad(s, s.a); // only keep for the sake of the compiler
    Vector3f grad_b = spring_constraint_grad(s, s.b); // only keep for the sake of the compiler
    numer += -spring_constraint(s);
    denom += particles[s.a].inv_mass * grad_a.norm() * grad_a.norm();
    denom += particles[s.b].inv_mass * grad_b.norm() * grad_b.norm();
    //// Your implementation ends

    // PBD if you comment out the following line
    denom += s.inv_stiffness / (dt * dt);
    
    if (denom == 0.0) return;
    float lambda = numer / denom;
    particles[s.a].pos += lambda * particles[s.a].inv_mass * grad_a;
    particles[s.b].pos += lambda * particles[s.b].inv_mass * grad_b;
}

/////////////////////////////////////////////////////
//// Step 4: Computing the collision constraint
//// If two particles a,b are closer than collision_dist,
//// a spring constraint is applied to separate them.
//// The rest length of the spring is set to collision_dist.
//// Otherwise return 0.0.
/////////////////////////////////////////////////////
float collision_constraint(Vector3f a, Vector3f b, float collision_dist){
    // Compute the distance between two particles a and b.
    // The constraint is defined as L - L0, where L is the current distance between a and b
    // and L0 = collision_dist is the minimum distance between a and b.

    float dist = (a - b).norm();
    if(dist < collision_dist){
        //// Your implementation starts
        return dist - collision_dist;
        //// Your implementation ends
    }
    else{
        return 0.0;
    }
}

/////////////////////////////////////////////////////
//// Step 5: Computing the collision constraint gradient
//// If two particles a,b are closer than collision_dist,
//// calculate the gradient of the collision constraint with respect to a.
//// It's similar to the spring constraint gradient.
//// Otherwise return vec2(0.0, 0.0).
/////////////////////////////////////////////////////
Vector3f collision_constraint_gradient(Vector3f a, Vector3f b, float collision_dist){
    // Compute the gradient of the collision constraint with respect to a.

    float dist = (a - b).norm();
    if(dist <= collision_dist){
        //// Your implementation starts
        if(dist == 0.0){
            return Vector3f(0.0, 0.0 ,0.0);
        }
        return (a-b)/dist;
        //// Your implementation ends
    }
    else{
        return Vector3f(0.0, 0.0, 0.0);
    }
}

/////////////////////////////////////////////////////
//// Step 6: Solving a single collision constraint
//// It solves for the collision constraint between particle i and j.
//// Calculate the numerator and denominator for the Lagrangian multiplier update.
//// You will calculate the numer/denom for PBD updates.
//// The Lagrangian multiplier update is calculated with lambda=-(numer/denom)
//// See the documentation for more details.
/////////////////////////////////////////////////////
void solve_collision_constraint(int i, int j, float collision_dist, float dt){
    // Compute the collision constraint for particles i and j.
    float numer = 0.0;
    float denom = 0.0;

    //// Your implementation starts
    Vector3f grad = collision_constraint_gradient(particles[i].pos,particles[j].pos,collision_dist); // only keep for the sake of the compiler
    numer += -collision_constraint(particles[i].pos,particles[j].pos,collision_dist);
    denom += particles[i].inv_mass * grad.norm() * grad.norm();
    denom += particles[j].inv_mass * grad.norm() * grad.norm();
    //// Your implementation ends

    //PBD if you comment out the following line, which is faster
    denom += (1. / 1000.) / (dt * dt);

    if (denom == 0.0) return;
    float lambda = numer / denom;
    particles[i].pos += lambda * particles[i].inv_mass * grad;
    particles[j].pos -= lambda * particles[j].inv_mass * grad;
}

float phi(Vector3f p){
    const float PI = 3.14159265359;
    //let's do sin(x)*sin(z)+0.5
    return p.y() - (0.1 * sin(p.x() * 2. * PI) * sin(p.z() * 2. * PI) - 0.5);
}

/////////////////////////////////////////////////////
//// Step 7: Computing the ground constraint
//// For a point p, if phi(p) < ground_collision_dist,
//// we set a constraint to push the point away from the ground.
//// The constraint is defined as phi(p) - ground_collision_dist.
//// Otherwise return 0.0.
/////////////////////////////////////////////////////
float ground_constraint(Vector3f p, float ground_collision_dist){
    if(phi(p) < ground_collision_dist){
        //// Your implementation starts
        return phi(p) - ground_collision_dist;
        //// Your implementation ends
    }
    else{
        return 0.0;
    }    
}

/////////////////////////////////////////////////////
//// Step 8: Computing the ground constraint gradient
//// If phi(p) < ground_collision_dist, 
//// compute the gradient of the ground constraint.
//// Otherwise return vec2(0.0, 0.0).
/////////////////////////////////////////////////////
Vector3f ground_constraint_gradient(Vector3f p, float ground_collision_dist){
    // Compute the gradient of the ground constraint with respect to p.
    const float PI = 3.14159265359;
    if(phi(p) < ground_collision_dist){
        //// Your implementation starts
        float grad_x = -0.1 * 2.0 * PI * cos( 2.0 * PI * p.x()) * sin(2.0 * PI * p.z()) ;
        float grad_z = -0.1 * 2.0 * PI * cos( 2.0 * PI * p.z()) * sin(2.0 * PI * p.x()) ;
        return Vector3f(grad_x,1.0,grad_z);
        
        //// Your implementation ends
    }
    else{
        return Vector3f(0.0, 0.0, 0.0);
    }
}

/////////////////////////////////////////////////////
//// Step 9: Solving a single ground constraint
//// It solves for the ground constraint for particle i.
//// Calculate the numerator and denominator for the Lagrangian multiplier update.
//// You will calculate the numer/denom for PBD updates.
//// The Lagrangian multiplier update is calculated with lambda=-(numer/denom)
//// See the documentation for more details.
/////////////////////////////////////////////////////
void solve_ground_constraint(int i, float ground_collision_dist, float dt){
    // Compute the ground constraint for particle i.
    float numer = 0.0;
    float denom = 0.0;

    //// Your implementation starts
    Vector3f grad = ground_constraint_gradient(particles[i].pos,ground_collision_dist); // only keep for the sake of the compiler
    numer += -ground_constraint(particles[i].pos,ground_collision_dist);
    denom += particles[i].inv_mass * grad.norm() * grad.norm();
    //// Your implementation ends

    //PBD if you comment out the following line, which is faster
    denom += (1. / 1000.) / (dt * dt);

    if (denom == 0.0) return;
    float lambda = numer / denom;
    particles[i].pos += lambda * particles[i].inv_mass * grad;
}

/////////////////////////////////////////////////////
//// Step 10: Solving all constraints
//// You need to solve for all 3 types of constraints using previously defined functions:
//// 1. Spring constraints defined by springs[1] to springs[n_springs-1]
//// 2. Ground constraints for all particles (except the mouse particle 0).
//// 3. Collision constraints for all pairs of particles (except the mouse particle 0).
/////////////////////////////////////////////////////
void solve_constraints(float dt) {
    // Solve all constraints

    //// Your implementation starts
    //spring
    for (int i = 1; i < n_springs; i++){
        solve_spring(springs[i], dt);
    }

    //ground
    for (int i = 1; i < n_particles; i++){
        solve_ground_constraint(i,ground_collision_dist,dt);
    }
    
    //particles
    for (int i = 1; i < n_particles-1; i++){
        for(int j = i+1; j < n_particles; j++){
            solve_collision_constraint(i,j,collision_dist,dt);
        }
    }
    //// Your implementation ends
}

float dist_to_segment(Vector3f p, Vector3f a, Vector3f b) {
    Vector3f pa = p - a;
    Vector3f ba = b - a;
    // Compute the projection factor and clamp it between 0 and 1.
    float h = MathUtils::Clamp(pa.dot(ba) / ba.dot(ba), 0.0, 1.0);
    // Return the distance from p to the closest point on the segment.
    return (pa - h * ba).norm();
}