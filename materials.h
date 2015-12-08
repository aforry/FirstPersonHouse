typedef struct MaterialType {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat shininess;
} MaterialType;

// Material types
MaterialType red = { { 0.3f, 0.0f, 0.0f, 1.0f },
{ 0.6f, 0.0f, 0.0f, 1.0f },
{ 0.8f, 0.6f, 0.6f, 1.0f },
32.0f };

MaterialType pearl = { { 0.25f, 0.20725f, 0.20725f, 1.0f },
{ 1.0f, 0.829f, 0.829f, 1.0f },
{ 0.296648f, 0.296648f, 0.296648f, 1.0 },
0.088f };

MaterialType brass = { { 0.33f, 0.22f, 0.03f, 1.0f },
{ 0.78f, 0.57f, 0.11f, 1.0f },
{ 0.99f, 0.91f, 0.81f, 1.0f },
27.8f };

MaterialType brown = { { 0.25f, 0.148f, 0.06475f, 1.0f },
{ 0.4f, 0.2368f, 0.1036f, 1.0f },
{ 0.774597f, 0.458561f, 0.200621f, 1.0f },
76.8f };

MaterialType gold = { { 0.24725f, 0.1995f, 0.0745f, 1.0f },
{ 0.75164f, 0.60648f, 0.22648f, 1.0f },
{ 0.628281f, 0.555802f, 0.366065f, 1.0f },
0.4f };
						
void set_material(GLenum face, MaterialType *material)
{
	glMaterialfv(face,GL_AMBIENT,material->ambient);
	glMaterialfv(face,GL_DIFFUSE,material->diffuse);
	glMaterialfv(face,GL_SPECULAR,material->specular);
	glMaterialf(face,GL_SHININESS,material->shininess);
}
