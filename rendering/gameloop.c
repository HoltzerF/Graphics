#include <time.h>
#include <stdio.h>

int fps = 0;
int fpsTarget = 60;
int fpsMaxShow = 999;

int input(){
	return 0;
}

void update(float delta){
	fps = (int)(1.0 / delta + 0.5);
	if(fps < 0 || fps > fpsMaxShow){
		fps = fpsMaxShow;
	}
}

void render(){
	printf("%d\n", fps);
}

int main(){
	
	clock_t last = clock();
	float fpsTargetSec = 1.0 / (float)fpsTarget;
	float accumulator = 0;
	
	while(1){
		
		// timing
		// ============
		clock_t curr = clock();
		float delta = (float)(curr - last) / CLOCKS_PER_SEC;
		last = curr;
		accumulator += delta;
		
		// input
		// ============
		if(input()){
			break;
		}
		
		// updating
		// ============
		if(fpsTarget > 0){
			while(accumulator > fpsTargetSec){
				update(fpsTargetSec);
				accumulator -= fpsTargetSec;
			}
		}else{
			update(delta);
		}
		
		// rendering
		// ============
		render();
	}
	return 0;
}
