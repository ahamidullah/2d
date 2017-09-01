uint64_t ticks_per_sec;
uint64_t last_sim_time = 0;

Level level;

enum Entity_ID {
	player_entity,
	box_entity,
	num_entities
};

typedef int Entity_Index;

/*
#define IS_COLLIDER_MOVING(id) (id & ~0x1)
#define SET_COLLIDER_MOVING(id) (id | 0x1)
#define SET_COLLIDER_STATIC(id) (id | 0x0)
#define GET_COLLIDER_INDEX_FROM_ID(id) (id >> 1)
#define SET_COLLIDER_INDEX_IN_ID(id, index) ((index << 1) | IS_COLLIDER_MOVING(id))
*/

enum Collision_Pair {
	player_box_collision,
	num_collision_pairs
};

constexpr Collision_Pair
get_collision_pair(Entity_ID id1, Entity_ID id2)
{
	if (id1 == player_entity && id2 == box_entity)
		return player_box_collision;
}

void
collider_handle_player_box_collision()
{

}

constexpr int MAX_COLLIDERS = 256;

struct AABB {
	Vec2f position;
	Vec2f dimensions;
	Entity_ID entity_id;
	Entity_Index entity_index;
};

typedef AABB Static_AABB;

struct Moving_AABB : AABB {
	Vec2f velocity;
};

// Make a distinction between moving the collider and removing it.
// When adding a collider, add it either to the end or the first id available for reuse.
// When moving it, replace with the last collider and fixup the pointers. Add it to the other collider list (above).
// When removing it, add the id to some reuse list?

struct Moving_Colliders {
	int size = 0;
	Moving_AABB *id_lookup[MAX_COLLIDERS];
	Moving_AABB data[MAX_COLLIDERS];
} moving_colliders;

struct Static_Colliders {
	int size = 0;
	Static_AABB *id_lookup[MAX_COLLIDERS];
	Static_AABB data[MAX_COLLIDERS];
} static_colliders;

/*
enum Collision_Result_Type {
	collided,
	nothing,
	num_collision_results
};
enum Collider_Tag {
	normal,
	num_collider_tags
};
*/

typedef unsigned Collider_Flags;

struct Collision_Result {
	Entity_Index first_index;
	Entity_Index second_index;
	//Vec2f penetration;
};

struct Collision_Pairwise_Results {
	Collision_Result data[MAX_COLLIDERS];
	int size = 0;
};

struct Collision_Sim_Results {
	Collision_Pairwise_Results pairwise_results[num_collision_pairs];
};

Collider_ID
add_collider(Vec2f pos, Vec2f dim, Entity_ID eid, Entity_Index index, Static_Colliders *sc)
{
	Collider_ID cid = sc->size;
	sc->data[sc->size] = { pos, dim, eid, index };
	sc->id_lookup[sc->size] = &sc->data[sc->size];
	++sc->size;
	return cid;
}

Moving_AABB
make_moving_from_static(Static_AABB *s, Vec2f velocity)
{
	Moving_AABB m;
	m.position = s->position;
	m.dimensions = s->dimensions;
	m.entity_id = s->entity_id;
	m.entity_index = s->entity_index;
	m.velocity = velocity;
	return m;
}

/*
void
remove_collider(Collider_Table *ct, Collider_ID id)
{
	ct->id_lookup[id] = NULL:
	// Add the 
	ct->id_lookup[id] = free_head;
	free_head = &ct->id_lookup[id];
	--ct->size;

}
*/

void
change_velocity(Collider_ID id, Vec2f velocity)
{
	if (static_colliders.id_lookup[id] != NULL) { // Move the collider into moving_colliders.
		assert(moving_colliders.id_lookup[id] == NULL);
		Static_AABB *s = static_colliders.id_lookup[id];
		moving_colliders.data[moving_colliders.size] = make_moving_from_static(s, velocity);
		moving_colliders.id_lookup[id] = &moving_colliders.data[moving_colliders.size];
		++moving_colliders.size;

		size_t last_collider_index = static_colliders.size - 1;
		assert(static_colliders.size > 0);
		// Remove the static collider.
		*s = static_colliders.data[last_collider_index];
		// Fix up the pointers.
		static_colliders.id_lookup[last_collider_index] = &static_colliders.data[id];
		static_colliders.id_lookup[id] = NULL;
		--static_colliders.size;
	}

	assert(moving_colliders.id_lookup[id] != NULL);
	Moving_AABB *m = moving_colliders.id_lookup[id];
	m->velocity = velocity;
}

/*
void
add_prospective_move(Collider_ID cid, Vec2f vel, Entity_ID eid, Prospective_Moves *pm)
{
	pm->data[pm->size].collider_id = cid;
	pm->data[pm->size].velocity = vel;
	pm->data[pm->size].entity_id = eid;
	++pm->size;
}
*/

void
sim_init()
{
	ticks_per_sec = SDL_GetPerformanceFrequency();

	moving_colliders.size = 0;
	memset(moving_colliders.data, 0, sizeof(moving_colliders.data));
	memset(moving_colliders.id_lookup, 0, sizeof(moving_colliders.id_lookup));

	static_colliders.size = 0;
	memset(static_colliders.data, 0, sizeof(static_colliders.data));
	memset(static_colliders.id_lookup, 0, sizeof(static_colliders.id_lookup));

	the_collider = { 200, 200, 100, 100 };

	Collider_ID id = add_collider({ 200.0f, 200.0f }, { 100.0f, 100.0f }, player_entity, 0, &static_colliders);
	add_collider({ 500.0f, 200.0f }, { 100.0f, 100.0f }, box_entity, 0, &static_colliders);

	change_velocity(id, { 1.0f, 0.0f });
/*
	moving_colliders.data[0].position = { 400.0f, 200.0f };
	moving_colliders.data[0].dimensions = { 100.0f, 100.0f };
	moving_colliders.data[0].entity_id = player_entity;
	moving_colliders.data[0].index = 0;
	moving_colliders.data[0].velocity = { 1.0f, 0.0f };
	moving_colliders.size = 1;

	static_colliders.data[0].position = { 500.0f, 200.0f };
	static_colliders.data[0].dimensions = { 100.0f, 100.0f };
	static_colliders.data[0].entity_id = box_entity;
	static_colliders.data[0].index = 0;
	static_colliders.size = 1;
*/
}

SDL_Rect
get_dest(Player *p)
{
	SDL_Rect frame = p->animation.info.frames[p->animation.cur_frame];
	return {
		p->transform.position.x,
		p->transform.position.y,
		frame.w * p->transform.scale,
		frame.h * p->transform.scale,
	};
}

/*
Collision_Result
handle_collision(AABB_Test c1, AABB c2, Collider_Flags f1, Collision_Flags f2)
{
	
}
*/

Collision_Sim_Results
collision_sim(Moving_Colliders *mc, Static_Colliders *sc)
{
	Collision_Sim_Results csr;
	//Vec2f penetration = { 0.0f, 0.0f };
	memset(csr.pairwise_results, 0, sizeof(csr.pairwise_results));

	for (int i = 0; i < mc->size; ++i) {
		Moving_AABB *t1 = &mc->data[i];
		AABB new_t1 = { t1->position + t1->velocity, t1->dimensions };
		for (int j = 0; j < sc->size; ++j) {
			Static_AABB *t2 = &sc->data[j];
			float new_t1_right_extant = new_t1.position.x + new_t1.dimensions.x;
			float t2_right_extant = t2->position.x + t2->dimensions.x;
			float new_t1_bottom_extant = new_t1.position.y + new_t1.dimensions.y;
			float t2_bottom_extant = t2->position.y + t2->dimensions.y;

			bool collided = false;

			if (new_t1_right_extant >= t2->position.x &&
			    new_t1_right_extant <= t2_right_extant)
				collided = true;
			else if (new_t1.position.x >= t2->position.x &&
			    new_t1.position.x <= t2_right_extant)
				collided = true;
				/*
			else if (new_t1_bottom_extant >= t2->position.y &&
			    new_t1_bottom_extant <= t2_bottom_extant)
				collided = true;
			else if (new_t1.position.y >= t2->position.y &&
			    new_t1.position.y <= t2_bottom_extant)
				collided = true;
				*/

			if (collided) {
				Collision_Pairwise_Results *results = &csr.pairwise_results[get_collision_pair(t1->entity_id, t2->entity_id)];
				results->data[results->size].first_index = t1->entity_index;
				results->data[results->size].second_index = t2->entity_index;
				//results->data[results->size].penetration = penetration;
				++results->size;

				// @TEMP
				//t1->velocity = { 0.0f, 0.0f };
			}
		}
		t1->position += t1->velocity;
	}
	return csr;
}

void
sim(bool *keys, Player *p)
{
	const double delta_time = last_sim_time == 0 ? 0.0 : (double)(SDL_GetPerformanceCounter() - last_sim_time) / ticks_per_sec;
	last_sim_time = SDL_GetPerformanceCounter();

	bool moved = keys[a_key] || keys[d_key] || keys[w_key] || keys[s_key];

	float acceleration = 0.1f;
	float min_speed = 0.0f, max_speed = 5.0f;

	if (moved) {
		if (keys[a_key])
			p->transform.velocity.x -= acceleration;
			//p->transform.position.x -= 1200.0*delta_time;
		if (keys[d_key])
			p->transform.velocity.x += acceleration;
			//p->transform.position.x += 1200.0*delta_time;
		if (keys[w_key])
			p->transform.velocity.y -= acceleration;
			//p->transform.position.y -= 1200.0*delta_time;
		if (keys[s_key])
			p->transform.velocity.y += acceleration;
			//p->transform.position.y += 1200.0*delta_time;

		if (p->animation.info.id != mc_walk_anim)
			p->animation = anim_get(mc_walk_anim);
	} else {
		p->transform.velocity = { 0.0f, 0.0f };
		if (p->animation.info.id != mc_idle_anim)
			p->animation = anim_get(mc_idle_anim);
	}

	p->transform.velocity = clamp_vec(p->transform.velocity, 0.0f, max_speed);

	//the_collider.x += 1.0f;

	//moving_colliders.data[0].position.x += 1.0f;

	Collision_Sim_Results csr = collision_sim(&moving_colliders, &static_colliders);

	p->transform.position.x = 0.0f;
	for (int i = 0; i < csr.pairwise_results[player_box_collision].size; ++i) {
		p->transform.position.x = 2.0f;
	}

	printf("%f\n", moving_colliders.data[0].velocity.x);
	the_collider.x += moving_colliders.data[0].velocity.x;
	//collision_test(colliders, collision_results);
	//p->transform.position += p->transform.velocity*delta_time;
	//p->collider.position.x += p->transform.velocity.x*delta_time;
	//p->collider.position.y += p->transform.velocity.y*delta_time;

/*
	for (int i = 0; i < level.rows; ++i) {
		for (int j = 0; j < level.cols; ++j) {
			SDL_Rect dest = { j*level.tile_width, i*level.tile_height, level.tiles[i][j].w, level.tiles[i][j].h };
			render_add_command(level.texture, level.tiles[i][j], dest);
		}
	}
*/
	render_add_command(p->animation.info.texture, anim_get_frame(&p->animation), p->transform.position, p->height, p->animation.info.frames[0].w / p->animation.info.frames[0].h);
}

void
sim_quit()
{

}

