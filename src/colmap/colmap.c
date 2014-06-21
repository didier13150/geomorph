#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

//	Generates a random POVRAY colormap
//	Syntax:  colmap <steps> <random_var_between_steps (0-100%)> 
//	<random_density_var> <random_color_var_red (0-100%)> <"""_green> <"""blue>
int main(int argc, char *argv[]) {

int steps = 10, i, var, rvar, gvar, bvar, dvar;
float incr, keyval, v, r, g, b, randomd, randomv;
if (argc>1)
	steps =atoi(argv[1]);
if (!steps)
	steps = 10;
if (argc>2)
	var = atoi(argv[2]);
else
	var = 100;
if (argc>3)
	dvar = atoi(argv[3]);
else
	dvar = 20;
if (argc>4)
	rvar = atoi(argv[4]);
else
	rvar = 10;
if (argc>5)
	gvar = atoi(argv[5]);
else
	gvar = 10;
if (argc>6)
	bvar = atoi(argv[6]);
else
	bvar = 10;
incr = (float) 1.0 / (float) steps;
// printf("Steps: %d\n",steps);
printf("#declare rock_map = \ncolor_map {\n");
v = (float) ((float) var) / (float) 100.0;
// printf("V: %f\n",v);
for (i=0; i<steps; i++) {
	keyval = i * incr;
	randomv = v * (((float) (rand()%1000))/(float) (steps*1000.0));
// printf("RANDOMV: %f\n",randomv);
	keyval = keyval + randomv;
	r = (float) (rvar*(rand()%100)/(float) 10000.0);
	g = (float) (gvar*(rand()%100)/(float) 10000.0);
	b = (float) (bvar*(rand()%100)/(float) 10000.0);
	randomd = 1.0 - (float) (dvar*(rand()%100)/(float) 10000.0);
// printf("RANDOMD: %f\n",randomd);
	printf("[%4.3f color %4.3f*rock_color + <%4.3f,%4.3f,%4.3f>]\n",
		keyval,randomd,r,g,b);
}

printf("}\n");
return 0;
}

