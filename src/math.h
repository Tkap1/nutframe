

func s_v3 v3(float x, float y, float z)
{
	s_v3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

func s_v4 v4(float x, float y, float z, float w)
{
	s_v4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
}

template <typename t0, typename t1, typename t2, typename t3>
func s_v4 v4(t0 x, t1 y, t2 z, t3 w)
{
	s_v4 result;
	result.x = (float)x;
	result.y = (float)y;
	result.z = (float)z;
	result.w = (float)w;
	return result;
}

template <typename t>
func s_v4 v4(t v)
{
	s_v4 result;
	result.x = (float)v;
	result.y = (float)v;
	result.z = (float)v;
	result.w = (float)v;
	return result;
}


func s_v4 v4(s_v3 v, float w)
{
	s_v4 result;
	result.x = v.x;
	result.y = v.y;
	result.z = v.z;
	result.w = w;
	return result;
}

func b8 rect_collides_circle(s_v2 rect_center, s_v2 rect_size, s_v2 center, float radius)
{
	b8 collision = false;

	float dx = fabsf(center.x - rect_center.x);
	float dy = fabsf(center.y - rect_center.y);

	if(dx > (rect_size.x/2.0f + radius)) { return false; }
	if(dy > (rect_size.y/2.0f + radius)) { return false; }

	if(dx <= (rect_size.x/2.0f)) { return true; }
	if(dy <= (rect_size.y/2.0f)) { return true; }

	float cornerDistanceSq = (dx - rect_size.x/2.0f)*(dx - rect_size.x/2.0f) +
													(dy - rect_size.y/2.0f)*(dy - rect_size.y/2.0f);

	collision = (cornerDistanceSq <= (radius*radius));

	return collision;
}


func float deg_to_rad(float d)
{
	return d * (pi / 180.f);
}

func float rad_to_deg(float r)
{
	return r * (180.f / pi);
}


func b8 circle_collides_circle(s_v2 center1, float radius1, s_v2 center2, float radius2)
{
	float dx = center2.x - center1.x;
	float dy = center2.y - center1.y;

	float distance = dx*dx + dy*dy;

	float combined_radius = radius1 + radius2;
	combined_radius *= combined_radius;

	if(distance <= combined_radius) { return true; }
	return false;
}

func s_v3 hsv_to_rgb(s_v3 colour)
{
	s_v3 rgb;

	if(colour.y <= 0.0f)
	{
		rgb.x = colour.z;
		rgb.y = colour.z;
		rgb.z = colour.z;
		return rgb;
	}

	colour.x *= 360.0f;
	if(colour.x < 0.0f || colour.x >= 360.0f)
		colour.x = 0.0f;
	colour.x /= 60.0f;

	u32 i = (u32)colour.x;
	float ff = colour.x - i;
	float p = colour.z * (1.0f - colour.y );
	float q = colour.z * (1.0f - (colour.y * ff));
	float t = colour.z * (1.0f - (colour.y * (1.0f - ff)));

	switch(i)
	{
	case 0:
		rgb.x = colour.z;
		rgb.y = t;
		rgb.z = p;
		break;

	case 1:
		rgb.x = q;
		rgb.y = colour.z;
		rgb.z = p;
		break;

	case 2:
		rgb.x = p;
		rgb.y = colour.z;
		rgb.z = t;
		break;

	case 3:
		rgb.x = p;
		rgb.y = q;
		rgb.z = colour.z;
		break;

	case 4:
		rgb.x = t;
		rgb.y = p;
		rgb.z = colour.z;
		break;

	default:
		rgb.x = colour.z;
		rgb.y = p;
		rgb.z = q;
		break;
	}

	return rgb;
}

template <typename t>
func t max(t a, t b)
{
	return a >= b ? a : b;
}

func b8 rect_collides_rect_topleft(s_v2 pos0, s_v2 size0, s_v2 pos1, s_v2 size1)
{
	return pos0.x + size0.x > pos1.x && pos0.x < pos1.x + size1.x &&
		pos0.y + size0.y > pos1.y && pos0.y < pos1.y + size1.y;
}

func b8 rect_collides_rect_center(s_v2 pos0, s_v2 size0, s_v2 pos1, s_v2 size1)
{
	s_v2 half_size0 = size0 / 2;
	s_v2 half_size1 = size1 / 2;
	float x_dist = fabsf(pos0.x - pos1.x);
	float y_dist = fabsf(pos0.y - pos1.y);
	return x_dist < half_size0.x + half_size1.x && y_dist < half_size0.y + half_size1.y;
}


[[nodiscard]]
func s_v2 random_point_in_rect_topleft(s_v2 pos, s_v2 size, s_rng* rng)
{
	return v2(
		pos.x + rng->randf32() * size.x,
		pos.y + rng->randf32() * size.y
	);
}


func s_v2 v2_rotated(s_v2 v, float angle)
{

	float x = v.x;
	float y = v.y;

	float cs = cosf(angle);
	float sn = sinf(angle);

	v.x = x * cs - y * sn;
	v.y = x * sn + y * cs;

	return v;
}
