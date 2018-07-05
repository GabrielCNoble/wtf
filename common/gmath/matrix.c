#include "matrix.h"
#include <intrin.h>


#ifdef __cplusplus
extern "C"
{
#endif

void CreatePerspectiveMatrix(mat4_t *mat,  float fovY, float aspect, float znear, float zfar, float x_shift, float y_shift, frustum_t *generated_frustum)
{
	//float tanHalfFovY=tan(fovY/2.0f);
	frustum_t f;
	int i, j;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			mat->floats[i][j]=0;
		}
	}
	 
	f.znear=znear;
	f.zfar=zfar;
	f.top=tan(fovY)*znear;
	f.bottom=-f.top;
	f.right=f.top*aspect;
	f.left=-f.right;
	
	/*f.right += x_shift;
	f.left += x_shift;
	
	f.top += y_shift;
	f.bottom += y_shift;*/
	
	//f.right = f.right - f.znear * (1.0 / tan(x_shift * 3.14159265));
	//f.left = f.left - f.znear * (1.0 / tan(x_shift * 3.14159265));
	
	//f.top = f.top - f.znear * (1.0 / tan(y_shift * 3.14159265));
	//f.bottom = f.bottom - f.znear * (1.0 / tan(y_shift * 3.14159265));

	mat->floats[0][0] = f.znear/f.right;
	mat->floats[1][1] = f.znear/f.top; 
	//mat->floats[2][2] = (-f.zfar)/(f.zfar-f.znear);
	mat->floats[2][2] = (-f.zfar+f.znear)/(f.zfar-f.znear);
	//mat->floats[2][2] = -1.0;
	
	
	mat->floats[3][2] = ((-2.0)*f.zfar*f.znear)/(f.zfar-f.znear);
	mat->floats[2][3] = -1.0;
	mat->floats[3][0] = 0.0;
	mat->floats[3][1] = 0.0;
	
	//printf("%f %f\n", f.znear, f.zfar);
	
	if(generated_frustum)
	{
		*generated_frustum=f;
	}
	
	return;
}

void Frustum(mat4_t *mat, float left, float right, float top, float bottom, float znear, float zfar, frustum_t *generated_frustum)
{
	frustum_t f;
	int i, j;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			mat->floats[i][j]=0;
		}
	}
	 
	float q = znear / zfar;
	
	f.zfar = zfar;
	f.znear = znear;
	f.top = top;
	f.bottom = bottom;
	f.left = left;
	f.right = right;
	
	mat->floats[0][0] = (2.0 * f.znear) / (f.right - f.left);
	mat->floats[1][1] = (2.0 * f.znear) / (f.top - f.bottom);
	mat->floats[2][2] = (-f.zfar+f.znear)/(f.zfar-f.znear);
	
	mat->floats[3][2] = ((-2.0)*f.zfar*f.znear)/(f.zfar-f.znear);
	mat->floats[2][3] = -1.0;
	mat->floats[2][0] = (f.right + f.left) / (f.right - f.left);
	mat->floats[2][1] = (f.top + f.bottom) / (f.top - f.bottom);
	
	//printf("%f %f\n", f.znear, f.zfar);
	
	if(generated_frustum)
	{
		*generated_frustum=f;
	}
	
	
}

void CreateOrthographicMatrix(mat4_t *mat, float left, float right, float top, float bottom, float znear, float zfar, frustum_t *generated_frustum)
{
	*mat=mat4_t_id();
	mat->floats[0][0]=2.0/(right-left);
	mat->floats[1][1]=2.0/(top-bottom);
	mat->floats[2][2]=-2.0/(zfar-znear);
	mat->floats[3][0]=-(right+left)/(right-left);
	mat->floats[3][1]=-(top+bottom)/(top-bottom);
	mat->floats[3][2]=-(zfar+znear)/(zfar-znear);
	return;
}

//########################################################################################################################
void mat4_t_rotate(mat4_t *mat, vec3_t axis, float angle, int b_set)
{
	int i, j;
	mat4_t result;
	vec3_t nAxis=normalize3(axis);
	vec3_t row1, row2, row3;
	float Sin, Cos;
	float o_m_cos;
	
	result=*mat;
	
	Sin=sin(3.14159265*angle);
	Cos=cos(3.14159265*angle);
	o_m_cos=1.0-Cos;
	
	
	if(!b_set)
	{
	
		result.floats[0][0]+=nAxis.floats[0]*nAxis.floats[0]*(o_m_cos)+Cos;
		result.floats[0][1]+=nAxis.floats[0]*nAxis.floats[1]*(o_m_cos)+nAxis.floats[2]*Sin;
		result.floats[0][2]+=nAxis.floats[0]*nAxis.floats[2]*(o_m_cos)-nAxis.floats[1]*Sin;
	
		result.floats[1][0]+=nAxis.floats[0]*nAxis.floats[1]*(o_m_cos)-nAxis.floats[2]*Sin;
		result.floats[1][1]+=nAxis.floats[1]*nAxis.floats[1]*(o_m_cos)+Cos;
		result.floats[1][2]+=nAxis.floats[1]*nAxis.floats[2]*(o_m_cos)+nAxis.floats[0]*Sin;
	
		result.floats[2][0]+=nAxis.floats[0]*nAxis.floats[2]*(o_m_cos)+nAxis.floats[1]*Sin;
		result.floats[2][1]+=nAxis.floats[1]*nAxis.floats[2]*(o_m_cos)-nAxis.floats[0]*Sin;
		result.floats[2][2]+=nAxis.floats[2]*nAxis.floats[2]*(o_m_cos)+Cos;
	
		row1=(vec3(result.floats[0][0], result.floats[0][1], result.floats[0][2]));
		row2=(vec3(result.floats[1][0], result.floats[1][1], result.floats[1][2]));
		row3=(vec3(result.floats[2][0], result.floats[2][1], result.floats[2][2]));
	
		result.floats[0][0]=row1.floats[0];
		result.floats[0][1]=row1.floats[1];
		result.floats[0][2]=row1.floats[2];
	
		result.floats[1][0]=row2.floats[0];
		result.floats[1][1]=row2.floats[1];
		result.floats[1][2]=row2.floats[2];
	
		result.floats[2][0]=row3.floats[0];
		result.floats[2][1]=row3.floats[1];
		result.floats[2][2]=row3.floats[2];
	}
	else
	{
		result.floats[0][0]=nAxis.floats[0]*nAxis.floats[0]*(o_m_cos)+Cos;
		result.floats[0][1]=nAxis.floats[0]*nAxis.floats[1]*(o_m_cos)+nAxis.floats[2]*Sin;
		result.floats[0][2]=nAxis.floats[0]*nAxis.floats[2]*(o_m_cos)-nAxis.floats[1]*Sin;
	
		result.floats[1][0]=nAxis.floats[0]*nAxis.floats[1]*(o_m_cos)-nAxis.floats[2]*Sin;
		result.floats[1][1]=nAxis.floats[1]*nAxis.floats[1]*(o_m_cos)+Cos;
		result.floats[1][2]=nAxis.floats[1]*nAxis.floats[2]*(o_m_cos)+nAxis.floats[0]*Sin;
	
		result.floats[2][0]=nAxis.floats[0]*nAxis.floats[2]*(o_m_cos)+nAxis.floats[1]*Sin;
		result.floats[2][1]=nAxis.floats[1]*nAxis.floats[2]*(o_m_cos)-nAxis.floats[0]*Sin;
		result.floats[2][2]=nAxis.floats[2]*nAxis.floats[2]*(o_m_cos)+Cos;
	}
	
	*mat=result;
	return;
}
//########################################################################################################################

//########################################################################################################################
void mat3_t_rotate(mat3_t *mat, vec3_t axis, float angle,  int b_set)
{
	int i, j;
	vec3_t nAxis=axis;
	vec3_t row1, row2, row3;
	float Sin, Cos;
	float c_m_one;
	mat3_t result;
	mat3_t temp;
	
	Sin=sin(3.14159265*angle); 
	Cos=cos(3.14159265*angle);
	
	c_m_one=1.0-Cos;
	
	
	if(!b_set)
	{		
		temp.floats[0][0]=nAxis.floats[0]*nAxis.floats[0]*(c_m_one)+Cos;
		temp.floats[0][1]=nAxis.floats[0]*nAxis.floats[1]*(c_m_one)+nAxis.floats[2]*Sin;
		temp.floats[0][2]=nAxis.floats[0]*nAxis.floats[2]*(c_m_one)-nAxis.floats[1]*Sin;
	
		temp.floats[1][0]=nAxis.floats[0]*nAxis.floats[1]*(c_m_one)-nAxis.floats[2]*Sin;
		temp.floats[1][1]=nAxis.floats[1]*nAxis.floats[1]*(c_m_one)+Cos;
		temp.floats[1][2]=nAxis.floats[1]*nAxis.floats[2]*(c_m_one)+nAxis.floats[0]*Sin;
	
		temp.floats[2][0]=nAxis.floats[0]*nAxis.floats[2]*(c_m_one)+nAxis.floats[1]*Sin;
		temp.floats[2][1]=nAxis.floats[1]*nAxis.floats[2]*(c_m_one)-nAxis.floats[0]*Sin;
		temp.floats[2][2]=nAxis.floats[2]*nAxis.floats[2]*(c_m_one)+Cos;
	
		row1=(vec3(temp.floats[0][0], temp.floats[0][1], temp.floats[0][2]));
		row2=(vec3(temp.floats[1][0], temp.floats[1][1], temp.floats[1][2]));
		row3=(vec3(temp.floats[2][0], temp.floats[2][1], temp.floats[2][2]));
	
		temp.floats[0][0]=row1.floats[0];
		temp.floats[0][1]=row1.floats[1];
		temp.floats[0][2]=row1.floats[2];
	
		temp.floats[1][0]=row2.floats[0];
		temp.floats[1][1]=row2.floats[1];
		temp.floats[1][2]=row2.floats[2];
	
		temp.floats[2][0]=row3.floats[0];
		temp.floats[2][1]=row3.floats[1];
		temp.floats[2][2]=row3.floats[2];
		
		mat3_t_mult(&result, mat, &temp);
	}
	else
	{
		result.floats[0][0]=nAxis.floats[0]*nAxis.floats[0]*(c_m_one)+Cos;
		result.floats[0][1]=nAxis.floats[0]*nAxis.floats[1]*(c_m_one)+nAxis.floats[2]*Sin;
		result.floats[0][2]=nAxis.floats[0]*nAxis.floats[2]*(c_m_one)-nAxis.floats[1]*Sin;
	
		result.floats[1][0]=nAxis.floats[0]*nAxis.floats[1]*(c_m_one)-nAxis.floats[2]*Sin;
		result.floats[1][1]=nAxis.floats[1]*nAxis.floats[1]*(c_m_one)+Cos;
		result.floats[1][2]=nAxis.floats[1]*nAxis.floats[2]*(c_m_one)+nAxis.floats[0]*Sin;
	
		result.floats[2][0]=nAxis.floats[0]*nAxis.floats[2]*(c_m_one)+nAxis.floats[1]*Sin;
		result.floats[2][1]=nAxis.floats[1]*nAxis.floats[2]*(c_m_one)-nAxis.floats[0]*Sin;
		result.floats[2][2]=nAxis.floats[2]*nAxis.floats[2]*(c_m_one)+Cos;
	}
	
	*mat=result;

	return;
}
//########################################################################################################################

void mat3_t_scale_x(mat3_t *mat, float scale)
{
	mat->floats[0][0] *= scale;
	mat->floats[0][1] *= scale;
	mat->floats[0][2] *= scale;
}

void mat3_t_scale_y(mat3_t *mat, float scale)
{
	mat->floats[1][0] *= scale;
	mat->floats[1][1] *= scale;
	mat->floats[1][2] *= scale;
}

void mat3_t_scale_z(mat3_t *mat, float scale)
{
	mat->floats[2][0] *= scale;
	mat->floats[2][1] *= scale;
	mat->floats[2][2] *= scale;
}

void mat2_t_rotate(mat2_t *mat, float angle, int b_set)
{
	float s;
	float c;
	s=sin(angle);
	c=cos(angle);
	mat2_t temp;
	mat2_t result;
	
	if(!b_set)
	{
		temp.floats[0][0]=c;
		temp.floats[0][1]=s;
		temp.floats[1][0]=-s;
		temp.floats[1][1]=c;
		
		mat2_t_mult(&result, &temp, mat);
		*mat=result;
		return;
	}
	else
	{
		mat->floats[0][0]=c;
		mat->floats[0][1]=s;
		mat->floats[1][0]=-s;
		mat->floats[1][1]=c;
	}
	return;
	
	
	
}

//########################################################################################################################

void mat4_t_scale(mat4_t *mat, vec3_t axis, float scale_factor)
{
	mat4_t m=*mat;
	float sf_m_one=scale_factor-1.0;
	
	mat->floats[0][0]=1.0+(sf_m_one)*axis.floats[0]*axis.floats[0];
	mat->floats[0][1]=(sf_m_one)*axis.floats[0]*axis.floats[1];
	mat->floats[0][2]=(sf_m_one)*axis.floats[0]*axis.floats[2];
	
	mat->floats[1][0]=(sf_m_one)*axis.floats[0]*axis.floats[1];
	mat->floats[1][1]=1.0+(sf_m_one)*axis.floats[1]*axis.floats[1];
	mat->floats[1][2]=(sf_m_one)*axis.floats[1]*axis.floats[2];
	
	mat->floats[2][0]=(sf_m_one)*axis.floats[0]*axis.floats[2];
	mat->floats[2][1]=(sf_m_one)*axis.floats[1]*axis.floats[2];
	mat->floats[2][2]=1.0+(sf_m_one)*axis.floats[2]*axis.floats[2];
	
	return;
}

void mat4_t_scale_axis_aligned(mat4_t *mat, vec3_t scale)
{
	mat->floats[0][0] *= scale.x;
	mat->floats[1][0] *= scale.x;
	mat->floats[2][0] *= scale.x;
	
	mat->floats[0][1] *= scale.y;
	mat->floats[1][1] *= scale.y;
	mat->floats[2][1] *= scale.y;
	
	mat->floats[0][2] *= scale.z;
	mat->floats[1][2] *= scale.z;
	mat->floats[2][2] *= scale.z;
}
//########################################################################################################################

//########################################################################################################################
void mat4_t_translate(mat4_t *mat, vec3_t position, int b_set)
{
	if(!b_set)
	{	
		mat->floats[3][0]+=position.floats[0];
		mat->floats[3][1]+=position.floats[1];
		mat->floats[3][2]+=position.floats[2];
	}
	else
	{
		mat->floats[3][0]=position.floats[0];
		mat->floats[3][1]=position.floats[1];
		mat->floats[3][2]=position.floats[2];
	}
	return;
}
//########################################################################################################################

//########################################################################################################################
mat4_t mat4_t_id()
{
	mat4_t result;

	result.floats[0][0]=1.0;
	result.floats[0][1]=0.0;
	result.floats[0][2]=0.0;
	result.floats[0][3]=0.0;
	
	result.floats[1][0]=0.0;
	result.floats[1][1]=1.0;
	result.floats[1][2]=0.0;
	result.floats[1][3]=0.0;
	
	result.floats[2][0]=0.0;
	result.floats[2][1]=0.0;
	result.floats[2][2]=1.0;
	result.floats[2][3]=0.0;
	
	result.floats[3][0]=0.0;
	result.floats[3][1]=0.0;
	result.floats[3][2]=0.0;
	result.floats[3][3]=1.0;

	return result;
}
//########################################################################################################################

//########################################################################################################################
mat3_t mat3_t_id()
{
	mat3_t result;
	result.floats[0][0]=1.0;
	result.floats[0][1]=0.0;
	result.floats[0][2]=0.0;
	
	result.floats[1][0]=0.0;
	result.floats[1][1]=1.0;
	result.floats[1][2]=0.0;
	
	result.floats[2][0]=0.0;
	result.floats[2][1]=0.0;
	result.floats[2][2]=1.0;

	return result;
}
//########################################################################################################################

mat2_t mat2_t_id()
{
	mat2_t result;
	result.floats[0][0]=1.0;
	result.floats[0][1]=0.0;
	result.floats[1][0]=0.0;
	result.floats[1][1]=1.0;
	return result;
}

//########################################################################################################################
void mat4_t_mult(mat4_t *result, mat4_t *mat1, mat4_t *mat2)
{

	int i;
	int j;
	int k;
	mat4_t t1; 
	mat4_t t2;
	
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			result->floats[i][j] =  mat1->floats[i][0] * mat2->floats[0][j];
			result->floats[i][j] += mat1->floats[i][1] * mat2->floats[1][j];
			result->floats[i][j] += mat1->floats[i][2] * mat2->floats[2][j];
			result->floats[i][j] += mat1->floats[i][3] * mat2->floats[3][j];
		}
	}
	return;
}
//########################################################################################################################


__fastcall void mat4_t_mult_fast(mat4_t *result, mat4_t *mat1, mat4_t *mat2)
{
	asm
	(
		
		/* result addr is in ebx... */
		"mov ebx, %[result]\n"
		/* mat1's addr is in esi... */
		"mov esi, %[mat1]\n"
		/* mat2's addr is in edi... */
		"mov edi, %[mat2]\n"
		
		//".intel_syntax noprefix\n"
		
		"mov ecx, 4\n"
		
		/* store mat2 in registers...  */
		"movups  xmm0, [edi]\n"
		"movups  xmm1, [edi + 16]\n"
		"movups  xmm2, [edi + 32]\n"
		"movups  xmm3, [edi + 48]\n"
		
		"_mat4_t_mult_fast_internal_loop:\n"
		
			/*move a ROW of mat1 into a register...*/
			"movups xmm4, [esi]\n"
			"add esi, 16\n"
			"dec ecx\n"
		
			/* copy the current row of mat1 to
			another register... */
			"movups xmm5, xmm4\n"
			
			/* copy the first element over
			the others...  */
			"shufps xmm5, xmm5, 0\n"
			
			/* multiply the elements
			of the first row of mat2...*/
			"mulps xmm5, xmm0\n"
			"movups xmm6, xmm5\n"
		
		
			
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0x55\n"
			"mulps xmm5, xmm1\n"
			"addps xmm6, xmm5\n"
		
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0xaa\n"
			"mulps xmm5, xmm2\n"
			"addps xmm6, xmm5\n"
			
			"movups xmm5, xmm4\n"
			"shufps xmm5, xmm5, 0xff\n"
			//"shufps xmm4, xmm4, 0xff\n"
			//"mulps xmm4, xmm3\n"
			//"addps xmm6, xmm4\n"
			"mulps xmm5, xmm3\n"
			"addps xmm6, xmm5\n"
		
			"movups [ebx], xmm6\n"
			"add ebx, 16\n"
			"test ecx, ecx\n"
			
			"jnz _mat4_t_mult_fast_internal_loop\n"
		
		//".att_syntax prefix\n"
		:: [result] "m" (result), [mat1] "m" (mat1), [mat2] "m" (mat2) : "edi", "esi", "ebx", "ecx"
		
	);
}


void mat3_t_mult(mat3_t *result,  mat3_t *mat1,  mat3_t *mat2)
{

	result->floats[0][0]=mat1->floats[0][0]*mat2->floats[0][0] + mat1->floats[0][1]*mat2->floats[1][0] + mat1->floats[0][2]*mat2->floats[2][0];					
	result->floats[0][1]=mat1->floats[0][0]*mat2->floats[0][1] + mat1->floats[0][1]*mat2->floats[1][1] + mat1->floats[0][2]*mat2->floats[2][1];					
	result->floats[0][2]=mat1->floats[0][0]*mat2->floats[0][2] + mat1->floats[0][1]*mat2->floats[1][2] + mat1->floats[0][2]*mat2->floats[2][2];
	
	result->floats[1][0]=mat1->floats[1][0]*mat2->floats[0][0] + mat1->floats[1][1]*mat2->floats[1][0] + mat1->floats[1][2]*mat2->floats[2][0];					
	result->floats[1][1]=mat1->floats[1][0]*mat2->floats[0][1] + mat1->floats[1][1]*mat2->floats[1][1] + mat1->floats[1][2]*mat2->floats[2][1];					
	result->floats[1][2]=mat1->floats[1][0]*mat2->floats[0][2] + mat1->floats[1][1]*mat2->floats[1][2] + mat1->floats[1][2]*mat2->floats[2][2];
	
	result->floats[2][0]=mat1->floats[2][0]*mat2->floats[0][0] + mat1->floats[2][1]*mat2->floats[1][0] + mat1->floats[2][2]*mat2->floats[2][0];					
	result->floats[2][1]=mat1->floats[2][0]*mat2->floats[0][1] + mat1->floats[2][1]*mat2->floats[1][1] + mat1->floats[2][2]*mat2->floats[2][1];					
	result->floats[2][2]=mat1->floats[2][0]*mat2->floats[0][2] + mat1->floats[2][1]*mat2->floats[1][2] + mat1->floats[2][2]*mat2->floats[2][2];
				
	return;	
}

void mat2_t_mult(mat2_t *result, mat2_t *mat1, mat2_t *mat2)
{
	result->floats[0][0]=mat1->floats[0][0]*mat2->floats[0][0] + mat1->floats[0][1]*mat2->floats[1][0];
	result->floats[0][1]=mat1->floats[0][0]*mat2->floats[0][1] + mat1->floats[0][1]*mat2->floats[1][1];
	result->floats[1][0]=mat1->floats[1][0]*mat2->floats[0][0] + mat1->floats[1][1]*mat2->floats[1][0];
	result->floats[1][1]=mat1->floats[1][0]*mat2->floats[0][1] + mat1->floats[1][1]*mat2->floats[1][1];
	
	return;
}

//########################################################################################################################
void mat4_t_mat3_t(mat3_t *result, mat4_t *mat)
{
	result->floats[0][0]=mat->floats[0][0];
	result->floats[0][1]=mat->floats[0][1];
	result->floats[0][2]=mat->floats[0][2];
	
	result->floats[1][0]=mat->floats[1][0];
	result->floats[1][1]=mat->floats[1][1];
	result->floats[1][2]=mat->floats[1][2];
	
	result->floats[2][0]=mat->floats[2][0];
	result->floats[2][1]=mat->floats[2][1];
	result->floats[2][2]=mat->floats[2][2];
	
	return;
}
//########################################################################################################################


void mat2_t_mat3_t(mat2_t *in, mat3_t *out)
{
	out->floats[0][0]=in->floats[0][0];
	out->floats[0][1]=in->floats[0][1];
	out->floats[0][2]=0.0;
	
	out->floats[1][0]=in->floats[1][0];
	out->floats[1][1]=in->floats[1][1];
	out->floats[1][2]=0.0;
	
	out->floats[2][0]=0.0;
	out->floats[2][1]=0.0;
	out->floats[2][2]=1.0;
}

void mat4_t_mat2_t(mat4_t *in, mat2_t *out)
{
	out->floats[0][0]=in->floats[0][0];
	out->floats[0][1]=in->floats[0][1];
	
	out->floats[1][0]=in->floats[1][0];
	out->floats[1][1]=in->floats[1][1];
	
	return;
}

void mat3_t_mat2_t(mat3_t *in, mat2_t *out)
{
	out->floats[0][0]=in->floats[0][0];
	out->floats[0][1]=in->floats[0][1];
	
	out->floats[1][0]=in->floats[1][0];
	out->floats[1][1]=in->floats[1][1];
	
	return;
}

//########################################################################################################################
void mat4_t_transpose(mat4_t *mat)
{
	float temp;
	
	temp=mat->floats[0][1];
	mat->floats[0][1]=mat->floats[1][0];
	mat->floats[1][0]=temp;
	
	temp=mat->floats[0][2];
	mat->floats[0][2]=mat->floats[2][0];
	mat->floats[2][0]=temp;
	
	temp=mat->floats[0][3];
	mat->floats[0][3]=mat->floats[3][0];
	mat->floats[3][0]=temp;
	
	temp=mat->floats[1][2];
	mat->floats[1][2]=mat->floats[2][1];
	mat->floats[2][1]=temp;
	
	temp=mat->floats[1][3];
	mat->floats[1][3]=mat->floats[3][1];
	mat->floats[3][1]=temp;
	
	temp=mat->floats[2][3];
	mat->floats[2][3]=mat->floats[3][2];
	mat->floats[3][2]=temp;

	return;
		
}


void mat4_t_inverse_transform(mat4_t *mat)
{
	float temp;
	float tempx;
	float tempy;
	float tempz; 
	
	temp = mat->floats[0][1];
	mat->floats[0][1] = mat->floats[1][0];
	mat->floats[1][0] = temp;
	
	temp = mat->floats[0][2];
	mat->floats[0][2] = mat->floats[2][0];
	mat->floats[2][0] = temp;
	
	temp = mat->floats[1][2];
	mat->floats[1][2] = mat->floats[2][1];
	mat->floats[2][1] = temp;
	
	
	
	tempx = -mat->floats[3][0] * mat->floats[0][0] -
						 mat->floats[3][1] * mat->floats[1][0] -
						 mat->floats[3][2] * mat->floats[2][0];
						 
	tempy = -mat->floats[3][0] * mat->floats[0][1] -
						 mat->floats[3][1] * mat->floats[1][1] -
						 mat->floats[3][2] * mat->floats[2][1];
						 
	tempz = -mat->floats[3][0] * mat->floats[0][2] -
						 mat->floats[3][1] * mat->floats[1][2] -
						 mat->floats[3][2] * mat->floats[2][2];					 					 
	
	mat->floats[3][0] = tempx;
	mat->floats[3][1] = tempy;
	mat->floats[3][2] = tempz;

}


//########################################################################################################################


void mat4_t_inverse(mat4_t *mat)
{
	
	
	mat4_t cf;
	
	/*cf.floats[0][0] = mat->floats[1][1] * mat->floats[2][2] * mat->floats[3][3] + 
					  mat->floats[1][2] * mat->floats[2][3] * mat->floats[3][1] + 
					  mat->floats[1][3] * mat->floats[2][1] * mat->floats[3][2] - 
					  mat->;*/
	
	//float det = mat->floats[0][0] * mat->floats[1][1] * mat->floats[2][2] * mat->floats[3][3] + 
				//mat->floats[0][0] * mat->floats[1][2] * mat->floats[2][3] * mat->floats[3][1] +
				//mat->floats[0][0] * mat->floats[1][3] * mat->floats[2][1] * mat->floats[3][2] +
				
				//mat->floats[0][1] * mat->floats[1][0] * mat->floats[2][3] * mat->floats[3][2] + 
				//mat->floats[0][1] * mat->floats[1][2] * mat->floats[2][0] * mat->floats[3][3] +
				//mat->floats[0][1] * mat->floats[1][3] * mat->floats[2][2] * mat->floats[3][0] +
				
				//mat->floats[0][2] * mat->floats[1][0] * mat->floats[2][1] * mat->floats[3][3] + 
				//mat->floats[0][2] * mat->floats[1][1] * mat->floats[2][3] * mat->floats[3][0] +
				//mat->floats[0][2] * mat->floats[1][3] * mat->floats[2][0] * mat->floats[3][1] +
				
				//mat->floats[0][3] * mat->floats[1][1] * mat->floats[2][2] * mat->floats[3][3] + 
				//mat->floats[0][3] * mat->floats[1][2] * mat->floats[2][3] * mat->floats[3][1] +
				//mat->floats[0][3] * mat->floats[1][3] * mat->floats[2][1] * mat->floats[3][2] +
				
				
				
				/*mat->floats[0][0] * mat->floats[1][1] * mat->floats[2][2] * mat->floats[3][3] + 
				mat->floats[0][0] * mat->floats[1][2] * mat->floats[2][3] * mat->floats[3][1] +
				mat->floats[0][0] * mat->floats[1][3] * mat->floats[2][1] * mat->floats[3][2] +
				
				mat->floats[0][1] * mat->floats[1][1] * mat->floats[2][2] * mat->floats[3][3] + 
				mat->floats[0][1] * mat->floats[1][2] * mat->floats[2][3] * mat->floats[3][1] +
				mat->floats[0][1] * mat->floats[1][3] * mat->floats[2][1] * mat->floats[3][2] +
				
				mat->floats[0][2] * mat->floats[1][1] * mat->floats[2][2] * mat->floats[3][3] + 
				mat->floats[0][2] * mat->floats[1][2] * mat->floats[2][3] * mat->floats[3][1] +
				mat->floats[0][2] * mat->floats[1][3] * mat->floats[2][1] * mat->floats[3][2] +
				
				mat->floats[0][3] * mat->floats[1][1] * mat->floats[2][2] * mat->floats[3][3] + 
				mat->floats[0][3] * mat->floats[1][2] * mat->floats[2][3] * mat->floats[3][1] +
				mat->floats[0][3] * mat->floats[1][3] * mat->floats[2][1] * mat->floats[3][2] + */
}

//########################################################################################################################
void mat3_t_transpose(mat3_t *mat)
{

	float temp;

	temp=mat->floats[0][1];
	mat->floats[0][1]=mat->floats[1][0];
	mat->floats[1][0]=temp;
	
	temp=mat->floats[0][2];
	mat->floats[0][2]=mat->floats[2][0];
	mat->floats[2][0]=temp;
	
	temp=mat->floats[1][2];
	mat->floats[1][2]=mat->floats[2][1];
	mat->floats[2][1]=temp;

	return;
		
}
//########################################################################################################################

void mat2_t_transpose(mat2_t *mat)
{
	float temp;
	temp=mat->floats[0][1];
	mat->floats[0][1]=mat->floats[1][0];
	mat->floats[1][0]=temp;
	return;
}

void mat2_t_invert(mat2_t *mat)
{
	float f;
	f=(mat->floats[0][0]*mat->floats[1][1])-(mat->floats[0][1]*mat->floats[1][0]);
	mat->floats[0][0]/=f;
	mat->floats[0][1]/=f;
	mat->floats[1][0]/=f;
	mat->floats[1][1]/=f;
	
	
	return;
}


void mat4_t_compose(mat4_t *result, mat3_t *orientation, vec3_t position)
{
	mat4_t output;
	mat3_t ori;
	if(orientation)
	{
		result->floats[0][0]=orientation->floats[0][0];
		result->floats[0][1]=orientation->floats[0][1];
		result->floats[0][2]=orientation->floats[0][2];
		
		result->floats[1][0]=orientation->floats[1][0];
		result->floats[1][1]=orientation->floats[1][1];
		result->floats[1][2]=orientation->floats[1][2];
	
		result->floats[2][0]=orientation->floats[2][0];
		result->floats[2][1]=orientation->floats[2][1];
		result->floats[2][2]=orientation->floats[2][2];
		
		
		
	}
	else
	{
		result->floats[0][0]=1.0;
		result->floats[0][1]=0.0;
		result->floats[0][2]=0.0;
	
		result->floats[1][0]=0.0;
		result->floats[1][1]=1.0;
		result->floats[1][2]=0.0;
	
		result->floats[2][0]=0.0;
		result->floats[2][1]=0.0;
		result->floats[2][2]=1.0;
	}
	
	result->floats[3][0]=position.floats[0];
	result->floats[3][1]=position.floats[1];
	result->floats[3][2]=position.floats[2];
	
	result->floats[0][3]=0.0;
	result->floats[1][3]=0.0;
	result->floats[2][3]=0.0;
	result->floats[3][3]=1.0;

	return;
}

void mat4_t_compose2(mat4_t *result, mat3_t *orientation, vec3_t position, vec3_t scale)
{
	mat4_t output;
	mat3_t ori;
	if(orientation)
	{
		result->floats[0][0] = orientation->floats[0][0];
		result->floats[0][1] = orientation->floats[0][1];
		result->floats[0][2] = orientation->floats[0][2];
		
		result->floats[1][0] = orientation->floats[1][0];
		result->floats[1][1] = orientation->floats[1][1];
		result->floats[1][2] = orientation->floats[1][2];
	
		result->floats[2][0] = orientation->floats[2][0];
		result->floats[2][1] = orientation->floats[2][1];
		result->floats[2][2] = orientation->floats[2][2];
	}
	else
	{
		result->floats[0][0] = 1.0;
		result->floats[0][1] = 0.0;
		result->floats[0][2] = 0.0;
	
		result->floats[1][0] = 0.0;
		result->floats[1][1] = 1.0;
		result->floats[1][2] = 0.0;
	
		result->floats[2][0] = 0.0;
		result->floats[2][1] = 0.0;
		result->floats[2][2] = 1.0;
	}
	
	result->floats[3][0] = position.floats[0];
	result->floats[3][1] = position.floats[1];
	result->floats[3][2] = position.floats[2];
	
	result->floats[0][0] *= scale.x;
	result->floats[0][1] *= scale.x;
	result->floats[0][2] *= scale.x;
	
	result->floats[1][0] *= scale.y;
	result->floats[1][1] *= scale.y;
	result->floats[1][2] *= scale.y;
	
	result->floats[2][0] *= scale.z;
	result->floats[2][1] *= scale.z;
	result->floats[2][2] *= scale.z;
	
	result->floats[0][3] = 0.0;
	result->floats[1][3] = 0.0;
	result->floats[2][3] = 0.0;
	result->floats[3][3] = 1.0;

	return;
}

void mat3_t_compose(mat3_t *result, vec3_t vec)
{
	*result=mat3_t_id();
	result->floats[0][0]=vec.floats[0];
	result->floats[1][1]=vec.floats[1];
	result->floats[2][2]=vec.floats[2];
	return;
}

__fastcall void mat3_t_vec3_t_mult(mat3_t *mat, vec3_t *vec)
{
	vec3_t temp;
	
	temp.x = vec->x * mat->floats[0][0] + vec->y * mat->floats[1][0] + vec->z * mat->floats[2][0];
	temp.y = vec->x * mat->floats[0][1] + vec->y * mat->floats[1][1] + vec->z * mat->floats[2][1];
	temp.z = vec->x * mat->floats[0][2] + vec->y * mat->floats[1][2] + vec->z * mat->floats[2][2];
	
	*vec = temp; 
}


vec3_t MultiplyVector3(mat3_t *mat, vec3_t vec)
{
	vec3_t result;
	result.floats[0] = 0.0;
	result.floats[1] = 0.0;
	result.floats[2] = 0.0;
	int i;
	for(i=0; i<3; i++)
	{
		result.floats[0] += mat->floats[i][0] * vec.floats[i];
		result.floats[1] += mat->floats[i][1] * vec.floats[i];
		result.floats[2] += mat->floats[i][2] * vec.floats[i];
	}
	return result;
}


/*PEWAPI mat3_t MatrixCopy3(mat3_t *out, mat3_t *in)
{
	asm
	(
		"movl %0, %%edi\n"
		"movl %1, %%esi\n"
		".intel_syntax noprefix\n"
		"mov ecx, 9\n"
		"rep stosd\n"
		".att_syntax prefix"
		:: "mr" (out), "mr" (in)
	);
}*/

void MatrixCopy4(mat4_t *out, mat4_t *in)
{
	asm
	(	
		
		//"movl %0, %%edi\n"
		//"movl %1, %%esi\n"
		"mov edi, %0\n"
		"mov esi, %1\n"
		
		//".intel_syntax noprefix\n"
		"mov ecx, 16\n"
		"rep stosd\n"
		//".att_syntax prefix"
		:: "mr" (out), "mr" (in)
	);
}

void quat_to_mat3_t(mat3_t *out, quaternion_t *q)
{
	float x2 = q->x + q->x;
	float y2 = q->y + q->y;
	float z2 = q->z + q->z;
	
	{
		float xx2 = q->x * x2;
		float yy2 = q->y * y2;
		float zz2 = q->z * z2;
		
		out->floats[0][0] = 1.0 - yy2 - zz2;
		out->floats[1][1] = 1.0 - xx2 - zz2;
		out->floats[2][2] = 1.0 - xx2 - yy2;
	}
	{
		float yz2 = q->y * z2;
		float wx2 = q->w * x2;
		
		out->floats[1][2] = yz2 + wx2;
		out->floats[2][1] = yz2 - wx2;
	}
	{
		float xy2 = q->x * y2;
		float wz2 = q->w * z2;
		
		out->floats[0][1] = xy2 + wz2;
		out->floats[1][0] = xy2 - wz2;
	}
	{
		float xz2 = q->x * z2;
		float wy2 = q->w * y2;
		
		out->floats[0][2] = xz2 - wy2;
		out->floats[2][0] = xz2 + wy2;
	}
}

/*void CreateYawPitchMatrix(mat3_t *r, float yaw, float pitch, vec3_t yaw_axis, vec3_t pitch_axis)
{
	
	mat3_t p;
	mat3_t y;
	
	r->floats[0][0]=1.0;
	r->floats[0][1]=0.0;
	r->floats[0][2]=0.0;
	
	r->floats[1][0]=0.0;
	r->floats[1][1]=1.0;
	r->floats[1][2]=0.0;
	
	r->floats[2][0]=0.0;
	r->floats[2][1]=0.0;
	r->floats[2][2]=1.0;
	
	y=RotateMatrix33f(NULL, yaw_axis, yaw, 0, 1);
	p=RotateMatrix33f(NULL, pitch_axis, pitch, 0, 1);
	
	MultiplyMatrix33f(r, &y, &p);
	
}*/


__fastcall void mat4_t_vec4_t_mult(mat4_t *mat, vec4_t *vec)
{
	asm
	(
		//"movl %0, %%edi\n"
		//"movl %1, %%esi\n"
		
		"mov edi, %0\n"
		"mov esi, %1\n"
		
		"movups xmm0, [edi]\n"
		"movups xmm1, xmm0\n"
		"movups xmm2, [esi]\n"
		"movups xmm3, [esi + 16]\n"
		"movups xmm4, [esi + 32]\n"
		"movups xmm5, [esi + 48]\n"
		
		"shufps xmm0, xmm0, 0\n"
		"mulps xmm2, xmm0\n"
		
		"movups xmm0, xmm1\n"
		"shufps xmm0, xmm0, 0x55\n"
		"mulps xmm3, xmm0\n"
		
		"movups xmm0, xmm1\n"
		"shufps xmm0, xmm0, 0xaa\n"
		"mulps xmm4, xmm0\n"
		
		"shufps xmm1, xmm1, 0xff\n"
		"mulps xmm5, xmm1\n"
		
		"addps xmm2, xmm3\n"
		"addps xmm2, xmm4\n"
		"addps xmm2, xmm5\n"
		
		"movups [edi], xmm2\n"
		
		
		: :"mr" (vec), "mr" (mat) : "esi", "edi"
	);
}

#ifdef __cplusplus
}
#endif







