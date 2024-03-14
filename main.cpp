#include<bits/stdc++.h>
#include<unistd.h>

using namespace std;

class Matrix { 
	// matrix[row][column]
	public:
	float matrix[3][3];
	
	Matrix(float matrix[3][3]){
		memcpy(&(this->matrix[0][0]), &matrix[0][0], 9*sizeof(float));
	}
	
	Matrix operator*(Matrix other) {
		float result[3][3];
		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 3; j++){
				result[i][j] = 0;
				for (int k = 0; k < 3; k++) {
					result[i][j] += matrix[i][k] * other.matrix[k][j];
				}
			}
		}
		return Matrix(result);
	}
};

class Some_matrices {
	public:
	static Matrix x_rotate_matrix(float phi){
		float table[3][3] = {{1, 0, 0},
							{0, cos(phi), sin(phi)},
							{0, -sin(phi), cos(phi)}};
		return Matrix(table);
	}
	
	static Matrix y_rotate_matrix(float phi){
		float table[3][3] = {{cos(phi), 0, sin(phi)},
							{0, 1, 0},
							{-sin(phi), 0, cos(phi)}};
		return Matrix(table);
	}
	
	static Matrix z_rotate_matrix(float phi){
		float table[3][3] = {{cos(phi), sin(phi), 0},
							{-sin(phi), cos(phi), 0},
							{0, 0, 1}};
		return Matrix(table);
	}
};


class Point {
	public:
	float x, y, z;
	
	Point(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	
	float module() {
		return sqrt(x * x + y * y + z * z);
	}

	void normalize() {
		float mod = module(); 
		
		if(mod == 0)
			return;
		
		x /= mod;
		y /= mod;
		z /= mod;
	}
	
	Point operator+(Point const& other) {
		return Point(x + other.x, y + other.y, z + other.z);
	}
	
	Point operator-(Point const& other) {
		return Point(x - other.x, y - other.y, z - other.z);
	}
	
	float operator*(Point const& other) {
		return x * other.x + y * other.y + z * other.z;
	}
	
	Point operator*(float constant){
		return Point(x * constant, y * constant, z * constant);
	}
	
	Point operator*(Matrix const& other) {
		return Point(x * other.matrix[0][0] + y * other.matrix[1][0] + z * other.matrix[2][0],
					x * other.matrix[0][1] + y * other.matrix[1][1] + z * other.matrix[2][1],
					x * other.matrix[0][2] + y * other.matrix[1][2] + z * other.matrix[2][2]);
	}	
};

std::ostream& operator<<(std::ostream& os, const Point& p){
    os << p.x << ' ' << p.y << ' ' << p.z;
    return os;
}

class Figure {
	public:
	std::function < Point( float, float ) > param_figure;
	
	Figure(){}
	
	Figure(std::function < Point( float, float ) > param_figure) {
		this->param_figure = param_figure;
	}
};

struct Point_to_dis {
	float z;
	Point orient_vec;
};

const Point light_dir(0, 0, -1);

class Pixel {
	
private:
	vector<Point_to_dis> z_buffer;
	static const int nr_of_chars = 11;
	//'$'
	char table_of_chars[nr_of_chars] = {'.',',','-','~',':',';','=','!','*','#','@'};
	
	Point get_lowest_z_point(){ 
		float minimal_z = z_buffer[0].z;
		Point winner = z_buffer[0].orient_vec;
		for(int i = 1; i < z_buffer.size(); i++) {
			if (z_buffer[i].z < minimal_z) {
				winner = z_buffer[1].orient_vec;
			}
		}
		return winner;
	}
	
public:
	void add_point_to_buffer(Point_to_dis p) {
		z_buffer.push_back(p);
	}
	
	char get_char() {
		if (z_buffer.empty()) {
			return ' ';
		}
		Point derivative_vec = get_lowest_z_point();
		derivative_vec.normalize();
		int light_intensity = round(abs(derivative_vec*light_dir)*(nr_of_chars - 1));
		
		return table_of_chars[light_intensity];
	}
	
	void clear_buffer() {
		z_buffer.clear();
	}
};

Point cross(Point v1, Point v2) {
	return Point(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

class Screen {
	
private:
	int size = 0;
	vector<vector<Pixel>> screen;
	float screen_dist, figure_dist;
	
public:
	Screen(){}
	
	Screen(const int n, float screen_dist) {
		this->screen_dist = screen_dist;
		
		size = n;
		screen.resize(n);
		generate(screen.begin(), screen.end(), 
				[=](){ 	vector<Pixel> line(size);
						generate(line.begin(), line.end(), [](){return Pixel();});
						return line;
				});
	}
	
	void calculate_pixels(float delta, std::function < Point( float, float ) > param_figure) {
		for (float t = 0; t <= 1; t += delta) {
			for (float r = 0; r <= 1; r += delta) {
				Point p = param_figure(t, r);

				float x_projection = screen_dist * p.x / p.z;
				float y_projection = screen_dist * p.y / p.z;
				
				int x_pixel = round((x_projection + 1)*(float)(size/2));
				int y_pixel = round((y_projection + 1)*(float)(size/2));
				
				if ( 0 > x_pixel || x_pixel >= size) continue;
				if ( 0 > y_pixel || y_pixel >= size) continue;
				
				float D = 0.0001;
				Point face_dir = cross(p - param_figure(t + D, r + D), p - param_figure(t - D, r + D));
				face_dir.normalize();
				
				screen[x_pixel][y_pixel].add_point_to_buffer({p.z, face_dir});
			}
		}
	}
	
	void show() {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				cout<<screen[j][size - i - 1].get_char();
			}
			cout<<"\n";
		}
		fflush(stdout);
	}
	
	void clear_screen() {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				screen[i][j].clear_buffer();
			}
		}
	}
	
	void clear_console() {
		system("cls");
	}
};

class System {
	private: 
	Screen screen;
	vector<Figure> figures;
	float figure_dist;
	
	public: 
	System(Screen screen, vector<Figure> figures, float figure_dist) { 
		this->figures = figures;
		this->screen = screen;
		this->figure_dist = figure_dist;
	}
	
	void run(int sleep_time_ms, float delta = 0.01, float x_rotate_speed = 0, float y_rotate_speed = 0, float z_rotate_speed = 0) {
		cout<<x_rotate_speed<<endl; fflush(stdout);
		
		float x_rot = 0;
		float y_rot = 0;
		float z_rot = 0;
		do{	
			screen.clear_screen();
			
			for(auto figure: figures) {
				std::function < Point( float, float ) > param_func = figure.param_figure;
				if (x_rot != 0) {
					param_func = [=](float a, float b){return param_func(a, b)*(Some_matrices::x_rotate_matrix(x_rot));};
				}
					
				if (y_rot != 0) {
					param_func = [=](float a, float b){return param_func(a, b)*(Some_matrices::y_rotate_matrix(y_rot));};
				}
					
				if (z_rot != 0) {
					param_func = [=](float a, float b){return param_func(a, b)*(Some_matrices::z_rotate_matrix(z_rot));};
				}
					
				param_func = [=](float a, float b){return param_func(a, b) + Point(0, 0, figure_dist);};
					
				screen.calculate_pixels(delta, param_func);
			}	
			
			screen.clear_console();
			screen.show(); 
			//usleep(sleep_time_ms*1000);
					
			x_rot += x_rotate_speed;
			y_rot += y_rotate_speed;
			z_rot += z_rotate_speed;
		}while(true);
	}
};

int main(){ 
	std::ios_base::sync_with_stdio(false);
	
	float screen_dist = 5;
	float figure_dist = 40;
	
	Figure torus([](float t, float r) {
		t *= 2*M_PI;// t, r \in [0,1]
		r *= 2*M_PI;
		float R1 = 1;
		float R2 = 4;
		Point p(R2 + R1*cos(t), R1*sin(t), 0);
		return (p*Some_matrices::y_rotate_matrix(r))*Some_matrices::x_rotate_matrix(M_PI/5);
	});

	Figure kulka_lewa([](float t, float r) {
		t *= 2*M_PI;// t, r \in [0,1]
		r *= 2*M_PI;
		float R = 2.5;
		Point shift(-2, -5, 0);
		Point p(R*cos(t), R*sin(t), 0);
		return (p*Some_matrices::y_rotate_matrix(r)) + shift;
	});

	Figure kulka_prawa([](float t, float r) {
		t *= 2*M_PI;// t, r \in [0,1]
		r *= 2*M_PI;
		float R = 2.5;
		Point shift(2, -5, 0);
		Point p(R*cos(t), R*sin(t), 0);
		return (p*Some_matrices::y_rotate_matrix(r)) + shift;
	});
	
	Figure walec([](float phi, float y) {
		phi *= 2*M_PI;
		float len = 8;
		float R = 1.6;
		y *= len;
		y -= len/2;
		return Point(R*sin(phi), y, R*cos(phi));
	});

	Figure kulka_gora([](float t, float r) {
		t *= 2*M_PI;// t, r \in [0,1]
		r *= 2*M_PI;
		float R = 1.6;
		Point shift(0, 4, 0);
		Point p(R*cos(t), R*sin(t), 0);
		return (p*Some_matrices::y_rotate_matrix(r)) + shift;
	});

	Figure serce_fig([](float t, float r) {
		t *= 2*M_PI;// t, r \in [0,1]
		r *= 2; r -= 1;
		float size = pow((1 + r)*(1 - r), 10)/3;
		float R = 1.6;
		Point p(size*16*(sin(t)*sin(t)*sin(t)), size*13*cos(t) - 5*cos(2*t) - 2*cos(3*t) - cos(4*t), r);
		return p;
	});

	vector<Figure> vector_torus{torus};
	vector<Figure> siur{kulka_lewa, kulka_prawa, walec, kulka_gora};
	vector<Figure> serce{serce_fig};

	Screen screen(50, screen_dist);
	
	System system(screen, siur, figure_dist);
	
	system.run(3000, 0.01, 0.3, 0.2);
}
