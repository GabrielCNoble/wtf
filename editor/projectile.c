#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GL\glew.h"

#include "projectile.h"
#include "physics.h"
#include "sound.h"


static int projectile_list_size;
static int projectile_count;
static int free_position_stack_top;
static int *free_position_stack;
static projectile_t *projectiles;

extern int pew_sounds[14];
extern int bullet_impact[3];

void projectile_Init()
{
	projectile_list_size = 512;
	projectile_count = 0;
	free_position_stack_top = -1;
	free_position_stack = malloc(sizeof(int) * projectile_list_size);
	projectiles = malloc(sizeof(projectile_t) * projectile_list_size);
}

void projectile_Finish()
{
	free(free_position_stack);
	free(projectiles);
}

int projectile_AddProjectile(vec3_t position, vec3_t delta, float radius, int life, int player_index)
{
	int projectile_index;
	projectile_t *projectile;
	
	
	if(free_position_stack_top >= 0)
	{
		projectile_index = free_position_stack[free_position_stack_top--];
	}
	else
	{
		projectile_index = projectile_count++;
		
		if(projectile_index >= projectile_list_size)
		{
			free(free_position_stack);
			free_position_stack = malloc(sizeof(int) * (projectile_list_size + 128));
			projectile = malloc(sizeof(projectile_t) * (projectile_list_size + 128));
			memcpy(projectile, projectiles, sizeof(projectile_t) * projectile_list_size);
			free(projectiles);
			projectiles = projectile;
			projectile_list_size += 128;
		}
	}
	
	projectile = &projectiles[projectile_index];
	
	if(radius > MAX_PROJECTILE_RADIUS) radius = MAX_PROJECTILE_RADIUS;
	else if(radius < MIN_PROJECTILE_RADIUS) radius = MIN_PROJECTILE_RADIUS;
	
	
	if(life > MAX_PROJECTILE_LIFE) life = MAX_PROJECTILE_LIFE;
	else if(life < MIN_PROJECTILE_LIFE) life = MIN_PROJECTILE_LIFE;
	
	
	projectile->position = position;
	projectile->delta = delta;
	//projectile->radius = 0xffff * (radius / MAX_PROJECTILE_RADIUS);
	
	projectile->radius = radius;
	projectile->life = life;
	projectile->index = projectile_index;
	projectile->player_index = player_index;
	
	return projectile_index;
}

void projectile_RemoveProjectile(int projectile_index)
{
	if(projectile_index >= 0 && projectile_index < projectile_count)
	{
		if(projectiles[projectile_index].index > -1)
		{
			projectiles[projectile_index].index = -1;
			
			free_position_stack_top++;
			free_position_stack[free_position_stack_top] = projectile_index;
		}
	}
}

void projectile_UpdateProjectiles()
{
	int i;
	vec3_t normal;
	vec3_t delta;
	vec3_t position;
	vec3_t t;
	player_t *hit;
	float radius;
	float l;
	int r;
	int b_collided;
	for(i = 0; i < projectile_count; i++)
	{
		if(projectiles[i].index == -1) continue;
		
		projectiles[i].life--;
		
		if(!projectiles[i].life)
		{
			projectile_RemoveProjectile(i);
		}
		else
		{
			
			hit = NULL;
			
			//b_collided = 0;
			
			if(physics_CheckProjectileCollision(&projectiles[i], &position, &normal, &hit))
			{
				
				//b_collided = 1;
				
				glPointSize(8.0);
				glBegin(GL_POINTS);
				glColor3f(1.0, 1.0, 0.0);
				glVertex3f(position.x, position.y, position.z);
				glEnd();
				glPointSize(1.0);
				
				/*if(hit)
				{
					printf("bullet hit a player!\n");
				}*/
				
				
				delta = projectiles[i].delta;
				
			//	r = ((float)((rand()%1024) - 512) / 512.0);
				
				r = rand() % 3;
				
				sound_PlaySound(bullet_impact[r], position, 1.8);
			
			 	r = rand() % 1024;
				
				//t = normalize3(delta);
				
				//l = dot3(normal, t);
				
				//printf("%f\n", l);
				
				
				
				if(r > 800)
				{
					
					l = dot3(normal, delta) * 2.0;
				
					normal.x *= l;
					normal.y *= l;
					normal.z *= l;
					
					delta.x -= normal.x;
					delta.y -= normal.y;
					delta.z -= normal.z;
					
					projectiles[i].position = position;
					
					projectiles[i].delta = delta;
					
					r = rand()%14;
					
					sound_PlaySound(pew_sounds[r], position, 0.25);
					
					//projectiles[i].life -= 10;
				}
				else
				{
					projectile_RemoveProjectile(i);
				}
				
				
				
				continue;
				
				//
			}
			/*else
			{
				projectiles[i].position.x += projectiles[i].delta.x;
				projectiles[i].position.y += projectiles[i].delta.y;
				projectiles[i].position.z += projectiles[i].delta.z;
			}*/
			
		}
		
		
		
		//position = projectiles[i].position;
		//radius = projectiles[i].radius;
		
		glPointSize(4.0);
		glBegin(GL_POINTS);
		glVertex3f(projectiles[i].position.x, projectiles[i].position.y, projectiles[i].position.z);
		glEnd();
		glPointSize(1.0);
		
		
		glLineWidth(4.0);
		glBegin(GL_LINES);
		glColor3f(0.0, 0.0, 1.0);				
		glVertex3f(projectiles[i].position.x, projectiles[i].position.y, projectiles[i].position.z);
		glVertex3f(projectiles[i].position.x + projectiles[i].delta.x, projectiles[i].position.y + projectiles[i].delta.y, projectiles[i].position.z + projectiles[i].delta.z);
		glEnd();
		glLineWidth(1.0);
		
		
		/* add delta AFTER testing for collision, since the test returns the
		intersection point of the NEXT physics tic. Also, to avoid the projectile
		missing intersection after being spawned... */
		
		
		projectiles[i].position.x += projectiles[i].delta.x;
		projectiles[i].position.y += projectiles[i].delta.y;
		projectiles[i].position.z += projectiles[i].delta.z;

		
		
		
	}	
}







