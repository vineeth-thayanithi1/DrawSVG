#include "viewport.h"

#include "CMU462.h"

namespace CMU462 {

	void ViewportImp::set_viewbox(float centerX, float centerY, float vspan) {

		// Task 5 (part 2): 
		// Set normalized svg to normalized device coordinate transformation. Your input
		// arguments are defined as normalized SVG canvas coordinates.
		this->centerX = centerX;
		this->centerY = centerY;
		this->vspan = vspan;

		//Initializing svg_2_norm matrix
		svg_2_norm[1].x = 0;
		svg_2_norm[0].y = 0;
		svg_2_norm[0].z = 0;
		svg_2_norm[1].z = 0;
		svg_2_norm[2].z = 1;

		//Setting translation
		svg_2_norm[2].x = vspan-centerX;
		svg_2_norm[2].y = vspan-centerY;
		
		//Setting Scale
		svg_2_norm[0].x = 1/(2*vspan);
		svg_2_norm[1].y = 1/(2*vspan);

		//Setting svg_2_norm
		set_svg_2_norm(svg_2_norm);
		


}

void ViewportImp::update_viewbox( float dx, float dy, float scale ) { 
  
  this->centerX -= dx;
  this->centerY -= dy;
  this->vspan *= scale;
  set_viewbox( centerX, centerY, vspan );
}

} // namespace CMU462
