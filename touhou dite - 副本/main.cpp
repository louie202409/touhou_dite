/*
 * 游戏模板 - BMP图片支持版本
 * 修复Texture类的方法问题
 */

#include <SDL.h>
#include <SDL_mixer.h>
#include<bits/stdc++.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #undef main
#endif

// 游戏常量定义
const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 360;
const int WINDOW_POS_X = SDL_WINDOWPOS_CENTERED;
const int WINDOW_POS_Y = SDL_WINDOWPOS_CENTERED;
const char* WINDOW_TITLE = "Touhou Dite machines Indev 0.0.1";

// 声音采样率
const int SAMPLE_RATE = 22050;


// 颜色定义
struct Color
{
    Uint8 r,g,b;
    Color(Uint8 red=0,Uint8 green=0,Uint8 blue=0)
	{
		r=red;g=green;b=blue;
	}
};

const Color COLOR_BLACK(0, 0, 0);
const Color COLOR_WHITE(255, 255, 255);
const Color COLOR_RED(255, 0, 0);
const Color COLOR_GREEN(0, 255, 0);
const Color COLOR_BLUE(0, 0, 255);
const Color COLOR_YELLOW(255, 255, 0);
const Color COLOR_CYAN(0, 255, 255);
const Color COLOR_MAGENTA(255, 0, 255);
const Color COLOR_ORANGE(255, 165, 0);

// 纹理管理类（完整版）
class Texture
{
private:
    SDL_Texture* m_texture;
    int m_width;
    int m_height;
    
public:
    Texture()
	{
		m_texture=NULL;
		m_width=0;
		m_height=0;
	}
    
    ~Texture()
	{
        free();
    }
    
    void free()
	{
        if(m_texture)
		{
            SDL_DestroyTexture(m_texture);
            m_texture=NULL;
            m_width=0;
            m_height=0;
        }
    }
    
    // 从BMP文件加载纹理
    bool loadFromBMP(SDL_Renderer* renderer,const std::string& path)
	{
        free();
        
        // 加载BMP表面
        SDL_Surface* loadedSurface=SDL_LoadBMP(path.c_str());
        if(loadedSurface==NULL)
		{
            // 不输出错误，因为可能没有图片文件
            return false;
        }
        
        // 设置颜色键（透明色）- 将RGB(255,0,255)设置为透明
        SDL_SetColorKey(loadedSurface,SDL_TRUE,SDL_MapRGB(loadedSurface->format,255,0,255));
        
        // 创建纹理
        m_texture=SDL_CreateTextureFromSurface(renderer,loadedSurface);
        if(m_texture==NULL)
		{
            std::cerr<<"Create texture fail: "<<SDL_GetError()<<std::endl;
            SDL_FreeSurface(loadedSurface);
            return false;
        }
        
        m_width=loadedSurface->w;
        m_height=loadedSurface->h;
        SDL_FreeSurface(loadedSurface);
        
        return true;
    }
    
    // 渲染纹理（原始大小）
    void render(SDL_Renderer* renderer,int x,int y)
	{
        SDL_Rect renderQuad={x,y,m_width,m_height};
        SDL_RenderCopy(renderer,m_texture,NULL,&renderQuad);
    }
    
    // 渲染纹理（指定大小）
    void render(SDL_Renderer* renderer,int x,int y,int w,int h)
	{
        SDL_Rect renderQuad={x,y,w,h};
        SDL_RenderCopy(renderer,m_texture,NULL,&renderQuad);
    }
    
    // 渲染纹理（带旋转和缩放）
    void renderEx(SDL_Renderer* renderer,int x,int y,int w,int h,double angle,SDL_Point* center,SDL_RendererFlip flip)
	{
        SDL_Rect renderQuad={x, y, w, h};
        SDL_RenderCopyEx(renderer,m_texture,NULL,&renderQuad,angle,center,flip);
    }
    
    // 设置纹理透明度
    void setAlpha(Uint8 alpha)
	{
        if(m_texture)
		{
            SDL_SetTextureAlphaMod(m_texture,alpha);
        }
    }
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    SDL_Texture* getTexture() const { return m_texture; }
    bool isValid() const { return m_texture != NULL; }
};

// 文本声音数据结构
struct TextSound
{
    std::vector<short> samples;
    int frequency;
    int duration;
    TextSound()
	{
		frequency=440;
		duration=500;
	}
};

// 粒子特效
struct Particle
{
    float x,y;
    float vx,vy;
    float life;
    Color color;
    Particle(float px,float py,float pvx,float pvy,const Color& col)
	{
		x=px,y=py,vx=pvx,vy=pvy;
		life=1.0f,color=col;
	}
};

// 8x16 ASCII字库（部分常用字符）
/*
 * 增强版字体系统 - 支持缩放、旋转、颜色变换
 */

class AdvancedFont {
private:
    SDL_Texture* fontTexture;     // 字体纹理
    int charWidth;                 // 原始字符宽度（像素）
    int charHeight;                // 原始字符高度（像素）
    int charsPerRow;               // 每行字符数
    int totalChars;                // 总字符数
    SDL_Renderer* renderer;        // 渲染器引用
    
    // 字符位置缓存
    struct CharInfo {
        SDL_Rect srcRect;          // 源矩形（在字体纹理中的位置）
        SDL_Rect dstRect;          // 目标矩形（缩放后的位置）
        double angle;              // 旋转角度
        SDL_Point center;          // 旋转中心
        SDL_RendererFlip flip;     // 翻转效果
    };
    
public:
    AdvancedFont() : fontTexture(NULL), charWidth(0), charHeight(0), 
                     charsPerRow(0), totalChars(0), renderer(NULL) {}
    
    ~AdvancedFont() {
        if (fontTexture) {
            SDL_DestroyTexture(fontTexture);
        }
    }
    
    // 从BMP文件加载字体
    bool loadFont(SDL_Renderer* renderer, const std::string& fontFile, 
              int charW, int charH, int firstChar = 32, int lastChar = 127) {
    this->renderer = renderer;
    charWidth = charW;
    charHeight = charH;
    totalChars = lastChar - firstChar + 1;
    
    SDL_Surface* surface = SDL_LoadBMP(fontFile.c_str());
    if (!surface) {
        std::cerr << "无法加载字体文件: " << fontFile << std::endl;
        return false;
    }
    
    // 将黑色背景设置为透明
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0, 0));
    
    fontTexture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!fontTexture) {
        std::cerr << "创建字体纹理失败: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return false;
    }
    
    charsPerRow = surface->w / charWidth;
    SDL_FreeSurface(surface);
    
    std::cout << "字体加载成功: " << charW << "x" << charH << ", "
              << totalChars << "个字符" << std::endl;
    
    return true;
}
    
    // 绘制单个字符（支持缩放和颜色）
    void drawChar(char ch, int x, int y, 
                  float scaleX = 1.0f, float scaleY = 1.0f,
                  const Color& color = Color(255, 255, 255),
                  double angle = 0.0,
                  SDL_RendererFlip flip = SDL_FLIP_NONE) {
        
        if (!fontTexture || ch < 32 || ch > 127) return;
        
        int index = ch - 32;  // ASCII 32是空格
        if (index >= totalChars) return;
        
        // 计算源矩形
        int srcX = (index % charsPerRow) * charWidth;
        int srcY = (index / charsPerRow) * charHeight;
        SDL_Rect srcRect = {srcX, srcY, charWidth, charHeight};
        
        // 计算目标矩形（应用缩放）
        int dstW = static_cast<int>(charWidth * scaleX);
        int dstH = static_cast<int>(charHeight * scaleY);
        SDL_Rect dstRect = {x, y, dstW, dstH};
        
        // 设置颜色
        SDL_SetTextureColorMod(fontTexture, color.r, color.g, color.b);
        SDL_SetTextureAlphaMod(fontTexture, 255);
        
        // 计算旋转中心（字符中心）
        SDL_Point center;
        if (angle != 0) {
            center.x = dstW / 2;
            center.y = dstH / 2;
        }
        
        // 渲染
        SDL_RenderCopyEx(renderer, fontTexture, &srcRect, &dstRect, 
                        angle, (angle != 0) ? &center : NULL, flip);
    }
    
    // 绘制字符串（支持多种效果）
    void drawString(const std::string& text, int x, int y,
                   float scaleX = 1.0f, float scaleY = 1.0f,
                   const Color& color = Color(255, 255, 255),
                   int spacing = 0,           // 字符间距
                   double angle = 0.0,
                   SDL_RendererFlip flip = SDL_FLIP_NONE) {
        
        int currentX = x;
        int charSpacing = static_cast<int>(charWidth * scaleX) + spacing;
        
        for (size_t i = 0; i < text.length(); ++i) {
            drawChar(text[i], currentX, y, scaleX, scaleY, color, angle, flip);
            currentX += charSpacing;
        }
    }
    
    // 绘制居中的字符串
    void drawStringCentered(const std::string& text, int centerX, int y,
                           float scaleX = 1.0f, float scaleY = 1.0f,
                           const Color& color = Color(255, 255, 255)) {
        
        int textWidth = getStringWidth(text, scaleX);
        int startX = centerX - textWidth / 2;
        drawString(text, startX, y, scaleX, scaleY, color);
    }
    
    // 获取字符串宽度
    int getStringWidth(const std::string& text, float scaleX = 1.0f) {
        int charSpacing = static_cast<int>(charWidth * scaleX);
        return text.length() * charSpacing;
    }
    
    // 获取字符串高度
    int getStringHeight(float scaleY = 1.0f) {
        return static_cast<int>(charHeight * scaleY);
    }
    
    // 绘制带阴影的文字
    void drawStringWithShadow(const std::string& text, int x, int y,
                             float scaleX = 1.0f, float scaleY = 1.0f,
                             const Color& textColor = Color(255, 255, 255),
                             const Color& shadowColor = Color(0, 0, 0),
                             int shadowOffsetX = 2, int shadowOffsetY = 2) {
        // 绘制阴影
        drawString(text, x + shadowOffsetX, y + shadowOffsetY, 
                  scaleX, scaleY, shadowColor);
        // 绘制文字
        drawString(text, x, y, scaleX, scaleY, textColor);
    }
    
    // 绘制渐变色文字（通过逐个字符改变颜色）
    void drawStringGradient(const std::string& text, int x, int y,
                           float scaleX = 1.0f, float scaleY = 1.0f,
                           const Color& startColor = Color(255, 0, 0),
                           const Color& endColor = Color(0, 255, 0)) {
        
        int currentX = x;
        int charSpacing = static_cast<int>(charWidth * scaleX);
        size_t len = text.length();
        
        for (size_t i = 0; i < len; ++i) {
            // 计算渐变颜色
            float t = static_cast<float>(i) / (len - 1);
            Color color(
                static_cast<Uint8>(startColor.r * (1 - t) + endColor.r * t),
                static_cast<Uint8>(startColor.g * (1 - t) + endColor.g * t),
                static_cast<Uint8>(startColor.b * (1 - t) + endColor.b * t)
            );
            
            drawChar(text[i], currentX, y, scaleX, scaleY, color);
            currentX += charSpacing;
        }
    }
    
    // 绘制波浪效果文字
    void drawStringWave(const std::string& text, int x, int y,
                       float scaleX = 1.0f, float scaleY = 1.0f,
                       const Color& color = Color(255, 255, 255),
                       float amplitude = 5.0f, float frequency = 0.5f) {
        
        int currentX = x;
        int charSpacing = static_cast<int>(charWidth * scaleX);
        
        for (size_t i = 0; i < text.length(); ++i) {
            float offsetY = sin(static_cast<float>(i) * frequency) * amplitude;
            drawChar(text[i], currentX, y + static_cast<int>(offsetY), 
                    scaleX, scaleY, color);
            currentX += charSpacing;
        }
    }
    
    // 绘制缩放动画文字
    void drawStringPulse(const std::string& text, int x, int y,
                        const Color& color = Color(255, 255, 255),
                        Uint32 time = 0) {  // time 用于动画
        
        float scale = 1.0f + sin(time * 0.005f) * 0.2f;
        drawString(text, x, y, scale, scale, color);
    }
};

void int_to_chars(char* ch,long long x,int digits)
{
	for(int i=digits-1;i>=0;i--)
	{
		ch[i]=char(x%10+'0');
		x/=10;
	}
	ch[digits]='\0';
}
bool hit(double x1,double y1,double size1,double x2,double y2,double size2)
{
	if(abs(x1-x2)<size1/2+size2/2 && abs(y1-y2)<size1/2+size2/2)
	{
		return 1;
	}
	return 0;
}
// 游戏主类
class ConsoleGame
{
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool isRunning;
    bool soundEnabled;
    
    // 纹理
    Texture textures[10050];
    int textures_cnt;
    
    // 上次帧时间
    Uint32 lastFrameTime;
    
    AdvancedFont smallFont;   // 8x16小字体
    AdvancedFont largeFont;   // 16x32大字体
    AdvancedFont titleFont;   // 标题字体
    
    bool isFullscreen,shouldFullscreen;
    
    int windowWidth,windowHeight;//tmp var for saving window size during fullscreen
    
    int stage;//0=home,1=home with menu,101=1
    int stageticks;//time a stage lasts
    int cursorpos[1050];
    
    float delta_time,fps;//to show fps
    int delta_time_pos;
    
    // 移动方向标志
    bool move_left,move_right,move_up,move_down,move_shift,move_ctrl,move_z,move_x,move_esc;
    int movecd_left,movecd_right,movecd_up,movecd_down;
    
//    struct Enemy {
//        float x, y;
//        float speed;
//        Enemy(float px, float py, float pspeed = 100.0f) : x(px), y(py), speed(pspeed) {}
//    };
	
	// 游戏对象
	struct drop
	{
		double x,y;
		double speed;
		int type;
		drop()
		{
			x=160,y=160,speed=0,type=1;
		}
		drop(double x1,double y1,double speed1,int type1)
		{
			x=x1;
			y=y1;
			speed=speed1;
			type=type1;
		}
		void move()
		{
			y+=speed;
			speed+=0.025;
			speed=std::min(speed,1.5);
		}
		bool dead()
		{
			return y>360;
		}
	};
	struct bullet_good
	{
		double x,y;
		int type,damage;
		double speed,dir;
		bullet_good()
		{
			x=160,y=160,speed=0,dir=0,type=1,damage=1;
		}
		bullet_good(double x1,double y1,double speed1,double dir1,int damage1,int type1)
		{
			x=x1;
			y=y1;
			speed=speed1;
			dir=dir1;
			damage=damage1;
			type=type1;
		}
		bool dead()
		{
			return x<0 || x>360 || y<0 || y>360;
		}
	};
	struct enemy
	{
		double x,y;
		double speedx,speedy;
		int health;
		int type;
		enemy()
		{
			x=160,y=160,speedx=0,speedy=-1,type=1,health=1;
		}
		enemy(double x1,double y1,double speedx1,double speedy1,int hp1,int type1)
		{
			x=x1;
			y=y1;
			speedx=speedx1;
			speedy=speedy1;
			health=hp1;
			type=type1;
		}
		void move()
		{
			if(type==1)
			{
				x+=speedx;
				y+=speedy;
			}
		}
		bool dead()
		{
			return x<0 || x>360 || y<0 || y>360 || health<0;
		}
	};
    //game variables
    std::queue<drop> drops;
    std::queue<bullet_good> bullet_goods;
    std::queue<enemy> enemys;
    
    int setting_hp,setting_bomb;
    
    int hp,bomb,graze,power,point,lv;
    long long score,hiscore;
    double player_x,player_y;
    int level,boss_number,boss_card_number;
    int player_type,shoot_cd;
    double player_speed,player_size;
    int player_card,player_resistance;
    
	
    
public:
    ConsoleGame()
	{
        window=NULL;
        renderer=NULL;
        isRunning=true;
        soundEnabled=true;
        lastFrameTime=0;
        isFullscreen=false;
        srand(static_cast<unsigned int>(time(NULL)));
    }
    ~ConsoleGame()
	{
        cleanup();
    }
    
    bool init()
	{
		import_settings();
		setting_hp=2,setting_bomb=3;
		
        if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0)
		{
            std::cerr<<"SDL init fail: "<<SDL_GetError()<<std::endl;
            return false;
        }
        
        window=SDL_CreateWindow(WINDOW_TITLE,WINDOW_POS_X,WINDOW_POS_Y,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
        if(window==NULL)
		{
            std::cerr<<"Window creation fail: "<<SDL_GetError()<<std::endl;
            return false;
        }
        
        renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
        if(renderer==NULL)
		{
            std::cerr<<"Renderer creation fail: "<<SDL_GetError()<<std::endl;
            return false;
        }
        
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
//        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        
        // 尝试加载图片
        loadImages();
        
        if(Mix_OpenAudio(SAMPLE_RATE,MIX_DEFAULT_FORMAT,2,1024)<0)
		{
            std::cerr<<"SDL_mixer init fail: "<<Mix_GetError()<<std::endl;
            soundEnabled=false;
        }
        
        lastFrameTime=SDL_GetTicks();
        
        // 加载字体
        if (!smallFont.loadFont(renderer, "resource\\fonts\\font_8x16.bmp", 8, 16)) {
            std::cerr << "加载小字体失败" << std::endl;
        }
        
        if (!largeFont.loadFont(renderer, "resource\\fonts\\font_16x32.bmp", 16, 32)) {
            std::cerr << "加载大字体失败" << std::endl;
        }
        
        SDL_StopTextInput(); 
        
        stage=0;
        stageticks=0;
        move_up=move_down=move_left=move_right=move_z=move_x=move_esc=move_shift=move_ctrl=0;
    	movecd_left=movecd_right=movecd_up=movecd_down=0;
        cursorpos[1]=1;
        cursorpos[2]=2;
        cursorpos[3]=1;
        cursorpos[4]=1;
        
        hiscore=0;
        delta_time_pos=0;
        fps=1;
        return true;
    }
    void import_settings()
    {
    	std::ifstream in("settings.txt");
    	std::string description;
    	in>>description>>shouldFullscreen;
	}
    
    void toggleFullscreen()
	{
    if (isFullscreen) {
        // 退出全屏
        SDL_SetWindowFullscreen(window, 0);
        SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        isFullscreen = false;
        
        // 恢复逻辑分辨率
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    } else {
        // 获取当前显示器模式
        SDL_DisplayMode dm;
        SDL_GetCurrentDisplayMode(0, &dm);
        
        // 保存当前窗口状态
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        
        // 进入真全屏
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        isFullscreen = true;
        
        // 获取全屏后的实际大小
        int fullscreenWidth, fullscreenHeight;
        SDL_GetWindowSize(window, &fullscreenWidth, &fullscreenHeight);
        
        // 重新设置逻辑分辨率（保持游戏内容比例）
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        
        // 可选：设置视口以填充整个屏幕
        SDL_Rect viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.w = fullscreenWidth;
        viewport.h = fullscreenHeight;
        SDL_RenderSetViewport(renderer, &viewport);
    }
}
    
    bool load_pic(int x,char* path)
    {
    	if(textures[x].loadFromBMP(renderer,path))
    	{
    		return 1;
		}
    	textures[x].loadFromBMP(renderer,"resource\\pic\\err.bmp");
    	return 0;
	}
    void loadImages()
	{
		std:: ifstream in("resource\\pic\\paths.txt");
		in>>textures_cnt;
		for(int i=1;i<=textures_cnt;i++)
		{
			char path[500];
			in>>path;
			if(load_pic(i,path))
			{
				std::cout<<"Loaded "<<path<<std::endl;
			}
			else
			{
				std::cout<<"Fail to load "<<path<<std::endl;
			}
		}
		in.close();
    }
    
    void cleanup()
	{
		for(int i=1;i<=textures_cnt;i++)
		{
			textures[i].free();
		}
        
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        
        Mix_CloseAudio();
        SDL_Quit();
    }
    
    void drawRect(float x,float y,float w,float h,const Color& color)
	{
        SDL_Rect rect;
        rect.x=static_cast<int>(x);
        rect.y=static_cast<int>(y);
        rect.w=static_cast<int>(w);
        rect.h=static_cast<int>(h);
        SDL_SetRenderDrawColor(renderer,color.r,color.g,color.b,255);
        SDL_RenderFillRect(renderer,&rect);
    }
    
    TextSound loadTextSound(const std::string& filename)
	{
        TextSound sound;
        std::ifstream file(filename.c_str());
        
        if(!file.is_open())
		{
            return sound;
        }
        
        std::string line;
        if(std::getline(file, line))
		{
            std::stringstream ss(line);
            ss>>sound.frequency;
            ss.ignore();
            ss>>sound.duration;
        }
        
        file.close();
        
        int numSamples=(SAMPLE_RATE*sound.duration)/1000;
        if(numSamples<=0)
		{
			numSamples = 1000;
		}
        
        sound.samples.resize(numSamples);
        
        for(int i=0;i<numSamples;++i)
		{
            float period=static_cast<float>(SAMPLE_RATE)/sound.frequency;
            short sample=(static_cast<int>(i/period)%2)?16384:-16384;
            sound.samples[i]=sample;
        }
        
        return sound;
    }
    
    void playSound(const TextSound& sound)
	{
        if(!soundEnabled||sound.samples.empty())
		{
			return;
		}
        
        Mix_Chunk* chunk=Mix_QuickLoad_RAW(
            const_cast<Uint8*>(reinterpret_cast<const Uint8*>(&sound.samples[0])),
            sound.samples.size()*sizeof(short)
        );
        
        if(chunk)
		{
            Mix_PlayChannel(-1,chunk,0);
            Mix_FreeChunk(chunk);
        }
    }
    
//    void addParticles(float x, float y, const Color& color, int count = 5) {
//        for (int i = 0; i < count; ++i) {
//            float angle = (rand() % 360) * 3.14159f / 180.0f;
//            float speed = (rand() % 100 + 50) / 10.0f;
//            float vx = cos(angle) * speed;
//            float vy = sin(angle) * speed;
//            particles.push_back(Particle(x, y, vx, vy, color));
//        }
//    }
    
    void update(float deltaTime)
	{
//		std::cout<<stage<<std::endl;
		if(stage==0)//home
		{
			if(stageticks>=60)
			{
				if(move_z || move_x || move_esc)
				{
					stage=1;
					stageticks=0;
				}
			}
		}
		else if(stage==1)//home with menu
		{
			if(stageticks<0)
			{
				if(stageticks==-1)
				{
					stage=2;
					stageticks=0;
				}
			}
			int dir=0;
			if(!move_up)
			{
				if(movecd_up>=0 && movecd_up<=29)
				{
					dir--;
				}
				movecd_up=30;
			}
			else
			{
				movecd_up--;
				if(movecd_up==0)
				{
					movecd_up=6;
					dir--;
				}
			}
			if(!move_down)
			{
				if(movecd_down>=0 && movecd_down<=29)
				{
					dir++;
				}
				movecd_down=30;
			}
			else
			{
				movecd_down--;
				if(movecd_down==0)
				{
					movecd_down=6;
					dir++;
				}
			}
			cursorpos[1]+=dir;
			if(cursorpos[1]==0)
			{
				cursorpos[1]=8;
			}
			if(cursorpos[1]==9)
			{
				cursorpos[1]=1;
			}
			
			if(stageticks>=60 && (move_x || move_esc))
			{
				cursorpos[1]=8;
			}
			
			if(move_z && stageticks>=60)
			{
				if(cursorpos[1]==1)
				{
					stageticks=-30;//start
				}
				if(cursorpos[1]==8)//quit
				{
					isRunning=false;
					return;
				}
			}
		}
		else if(stage==2)
		{
			if(stageticks<0)
			{
				if(stageticks==-120)
				{
					stageticks=30;
					return;
				}
				else if(stageticks==-99)
				{
					stage=3;
					stageticks=0;
					return;
				}
				else if(stageticks==-1)
				{
					stage=0;
					stageticks=0;
					return;
				}
			}
			int dir=0;
			if(!move_up)
			{
				if(movecd_up>=0 && movecd_up<=29)
				{
					dir--;
				}
				movecd_up=30;
			}
			else
			{
				movecd_up--;
				if(movecd_up==0)
				{
					movecd_up=6;
					dir--;
				}
			}
			if(!move_down)
			{
				if(movecd_down>=0 && movecd_down<=29)
				{
					dir++;
				}
				movecd_down=30;
			}
			else
			{
				movecd_down--;
				if(movecd_down==0)
				{
					movecd_down=6;
					dir++;
				}
			}
			cursorpos[2]+=dir;
			if(cursorpos[2]==0)
			{
				cursorpos[2]=4;
			}
			if(cursorpos[2]==5)
			{
				cursorpos[2]=1;
			}
			
			if(stageticks>=60 && (move_x || move_esc))
			{
				stageticks=-30;
			}
			
			if(move_z && stageticks>=60)
			{
				stageticks=-100;
			}
		}
		else if(stage==3)
		{
			if(stageticks<0)
			{
				if(stageticks==-29)
				{
					stage=2;
					stageticks=-150;
					return;
				}
				else if(stageticks==-90)
				{
					stageticks=31;
					return;
				}
				else if(stageticks==-140)
				{
					stageticks=31;
					return;
				}
				else if(stageticks==-199)
				{
					stage=4;
					stageticks=0;
					return;
				}
			}
			int dir=0;
			if(!move_right)
			{
				if(movecd_up>=0 && movecd_up<=29)
				{
					dir--;
				}
				movecd_up=30;
			}
			else
			{
				movecd_up--;
				if(movecd_up==0)
				{
					movecd_up=6;
					dir--;
				}
			}
			if(!move_left)
			{
				if(movecd_down>=0 && movecd_down<=29)
				{
					dir++;
				}
				movecd_down=30;
			}
			else
			{
				movecd_down--;
				if(movecd_down==0)
				{
					movecd_down=6;
					dir++;
				}
			}
			cursorpos[3]+=dir;
			if(cursorpos[3]==0)
			{
				cursorpos[3]=2;
			}
			if(cursorpos[3]==3)
			{
				cursorpos[3]=1;
			}
			
			if(move_x || move_esc)
			{
				stageticks=-30;
			}
			if(dir==-1)
			{
				stageticks=-100;
			}
			else if(dir==1)
			{
				stageticks=-150;
			}
			if(move_z && stageticks>=30)
			{
				stageticks=-200;
			}
		}
		else if(stage==4)
		{
			if(stageticks<0)
			{
				if(stageticks==-20)
				{
					stage=3;
					stageticks=30;
					return;
				}
				else if(stageticks==-99)
				{
					stage=5;player_type=1;
					if(cursorpos[3]==2)
					{
						player_type+=2;
					}
					if(cursorpos[4]==2)
					{
						player_type+=1;
					}
					score=0;
					point=power=graze=0;
					hp=setting_hp;
					bomb=setting_bomb;
					player_size=2.5;
					player_x=160;
					player_y=330;
					lv=1;
					if(player_type==1)
					{
						player_resistance=330;
						player_speed=2;
					}
					else if(player_type==2)
					{
						player_resistance=210;
						player_speed=2;
					}
					else if(player_type==3)
					{
						player_resistance=300;
						player_speed=2.75;
					}
					else if(player_type==4)
					{
						player_resistance=360;
						player_speed=2.75;
					}
					stageticks=0;
				}
			}
			int dir=0;
			if(!move_up)
			{
				if(movecd_up>=0 && movecd_up<=29)
				{
					dir--;
				}
				movecd_up=30;
			}
			else
			{
				movecd_up--;
				if(movecd_up==0)
				{
					movecd_up=6;
					dir--;
				}
			}
			if(!move_down)
			{
				if(movecd_down>=0 && movecd_down<=29)
				{
					dir++;
				}
				movecd_down=30;
			}
			else
			{
				movecd_down--;
				if(movecd_down==0)
				{
					movecd_down=6;
					dir++;
				}
			}
			cursorpos[4]+=dir;
			if(cursorpos[4]==0)
			{
				cursorpos[4]=2;
			}
			if(cursorpos[4]==3)
			{
				cursorpos[4]=1;
			}
			if(stageticks>30 && (move_x || move_esc))
			{
				stageticks=-50;
			}
			if(stageticks>30 && (move_z))
			{
				stageticks=-100;
			}
		}
		else if(stage==5)
		{
			double dir_x=0,dir_y=0;
	        if(move_left)
			{
				dir_x-=player_speed;
			}
	        if(move_right)
			{
				dir_x=player_speed;
			}
	        if(move_up)
			{
				dir_y-=player_speed;
			}
	        if(move_down)
			{
				dir_y+=player_speed;
			}
	        
	        if(dir_x!=0 && dir_y!=0)
			{
	            dir_x*=0.7071;
	            dir_y*=0.7071;
	        }
	        if(move_shift)
			{
				dir_x*=0.5;
				dir_y*=0.5;
			}
	        player_x+=dir_x;
	        player_y+=dir_y;
	        
	        if(player_x-player_size/2<10) 
			{
				player_x=10+player_size/2;
			}
	        if(player_y-player_size/2<10) 
			{
				player_y=10+player_size/2;
			}
	        if(player_x+player_size/2>305)
			{
				player_x=305-player_size/2;
			}
	        if(player_y+player_size/2>345)
			{
				player_y=345-player_size/2;
			}
	        
	        if(stageticks%60==0)
			{
				drops.push(drop(rand()%310+20,rand()%100+20,-2,rand()%5+1));
			}
	        std::queue<drop> tmp;
	        while(!drops.empty())
	        {
	        	drop top=drops.front();
	        	drops.pop();
	        	top.move();
	        	if(hit(top.x,top.y,16,player_x,player_y,2.5))
	        	{
	        		if(top.type==1)
	        		{
	        			power+=1;
					}
					if(top.type==2)
					{
						score+=std::min(10000.0,(360-top.y)/270.0*10000);
						point++;
					}
					if(top.type==3)
					{
						power+=8;
					}
					if(top.type==4)
					{
						bomb++;
					}
					if(top.type==5)
					{
						hp++;
					}
	        		continue;
				}
	        	if(!top.dead())
	        	{
	        		tmp.push(top);
				}
			}
			while(!tmp.empty())
			{
				drops.push(tmp.front());
				tmp.pop();
			}
			
			if(move_z)
			{
				if(player_type==1)
				{
					if(0<=power && power<8)
					{
						if(stageticks%10==0)
						{
							bullet_goods.push(bullet_good(player_x,player_y,4,0,90,1));
						}
					}
					else if(8<=power && power<16)
					{
						if(stageticks%10==0)
						{
							bullet_goods.push(bullet_good(player_x,player_y,4,0,90,1));
							bullet_goods.push(bullet_good(player_x,player_y,4,-0.1,25,2));
							bullet_goods.push(bullet_good(player_x,player_y,4,0.1,25,2));
						}
					}
					else if(16<=power && power<32)
					{
						if(stageticks%10==0)
						{
							bullet_goods.push(bullet_good(player_x,player_y,4,1,90,1));
							bullet_goods.push(bullet_good(player_x,player_y,4,-1,90,1));
							bullet_goods.push(bullet_good(player_x,player_y,4,-0.1,25,2));
							bullet_goods.push(bullet_good(player_x,player_y,4,0.1,25,2));
						}
					}
					else if(32<=power && power<48)
					{
						if(stageticks%10==0)
						{
							bullet_goods.push(bullet_good(player_x,player_y,0,-4,90,1));
							bullet_goods.push(bullet_good(player_x,player_y,4,-0.1,25,2));
							bullet_goods.push(bullet_good(player_x,player_y,4,0.1,25,2));
							bullet_goods.push(bullet_good(player_x,player_y,4,-0.1,25,2));
							bullet_goods.push(bullet_good(player_x,player_y,4,0.1,25,2));
						}
					}
				}
				
			}
			std::queue<bullet_good> tmp2;
			while(!bullet_goods.empty())
			{
				bullet_good top=bullet_goods.front();
				bullet_goods.pop();
		       	if(top.type==1)
				{
					top.x+=sin(top.dir)*top.speed;
					top.y-=cos(top.dir)*top.speed;
				}
				else if(top.type==2)
				{
		        	std::queue<enemy> tmpp2;
			        if(!enemys.empty())
			        {
			        	enemy topp=enemys.front();
						int deltax=topp.x-top.x,deltay=topp.y-top.y;
						double speedleft=top.speed;
						if(deltax!=0)
						{
							if(abs(deltax)<speedleft)
							{
								top.x=topp.x;
								speedleft-=deltax;
							}
							else
							{
								if(deltax>0)
								{
									top.x+=speedleft;
								}
								else
								{
									top.x-=speedleft;
								}
							}
						}
						if(deltay!=0)
						{
							if(abs(deltay)<speedleft)
							{
								top.y=topp.y;
								speedleft-=deltay;
							}
							else
							{
								if(deltay>0)
								{
									top.y+=speedleft;
								}
								else
								{
									top.y-=speedleft;
								}
							}
						}
					}
					else
					{
						top.y-=top.speed;
					}
				}
				if(top.dead())
				{
					continue;
				}
				tmp2.push(top);
			}
			while(!tmp2.empty())
			{
				bullet_goods.push(tmp2.front());
				tmp2.pop();
			}
			
			if(stageticks%120==0)
			{
				enemys.push(enemy(rand()%310+20,rand()%100+20,0,1,100,1));
			}
			std::queue<enemy> tmp3;
			while(!enemys.empty())
			{
				enemy top=enemys.front();
				enemys.pop();
				top.move();
				std::queue<bullet_good> tmpp2;
				int dmg=0;
				while(!bullet_goods.empty())
				{
					bullet_good topp=bullet_goods.front();
					bullet_goods.pop();
					if(hit(topp.x,topp.y,8,top.x,top.y,16))
					{
						dmg+=topp.damage;
					}
					else
					{
						tmpp2.push(topp);
					}
				}
				while(!tmpp2.empty())
				{
					bullet_goods.push(tmpp2.front());
					tmpp2.pop();
				}
				if(hit(top.x,top.y,8,player_x,player_y,8))
				{
					hp--;
					top.health=-1;
				}
				top.health-=dmg;
				if(top.dead())
				{
					drops.push(drop(top.x,top.y,-2,rand()%3+1));
					continue;
				}
				tmp3.push(top);
			}
			while(!tmp3.empty())
			{
				enemys.push(tmp3.front());
				tmp3.pop();
			}
//	        static float enemySpawnTimer = 0.0f;
//	        enemySpawnTimer += deltaTime;
//	        
//	        float spawnDelay = std::max(0.5f, 1.0f - score / 100.0f);
//	        if (enemySpawnTimer >= spawnDelay) {
//	            enemySpawnTimer = 0.0f;
//	            float enemyX = static_cast<float>(rand() % (SCREEN_WIDTH - static_cast<int>(ENEMY_SIZE)));
//	            float enemySpeed = 80.0f + score / 20.0f;
//	            enemies.push_back(Enemy(enemyX, 0, enemySpeed));
//	        }
//	        std::vector<Enemy>::iterator it = enemies.begin();
//	        while (it != enemies.end()) {
//	            it->y += it->speed * deltaTime;
//	            
//	            if (playerX < it->x + ENEMY_SIZE && playerX + PLAYER_SIZE > it->x &&
//	                playerY < it->y + ENEMY_SIZE && playerY + PLAYER_SIZE > it->y) {
//	                isRunning = false;
//	                break;
//	            }
//	            
//	            if (it->y > SCREEN_HEIGHT) {
//	                float enemyX = it->x;
//	                it = enemies.erase(it);
//	                score++;
//	                addParticles(enemyX + ENEMY_SIZE/2, SCREEN_HEIGHT, COLOR_GREEN, 3);
//	                
//	                TextSound beep;
//	                beep.frequency = 440 + score * 10;
//	                beep.duration = 80;
//	                playSound(beep);
//	            } else {
//	                ++it;
//	            }
//	        }
	        
//	        std::vector<Particle>::iterator pit = particles.begin();
//	        while (pit != particles.end()) {
//	            pit->x += pit->vx * deltaTime;
//	            pit->y += pit->vy * deltaTime;
//	            pit->life -= deltaTime * 2.0f;
//	            
//	            if (pit->life <= 0 || pit->x < 0 || pit->x > SCREEN_WIDTH || 
//	                pit->y < 0 || pit->y > SCREEN_HEIGHT) {
//	                pit = particles.erase(pit);
//	            } else {
//	                ++pit;
//	            }
//	        }
		}
		delta_time+=deltaTime;
		delta_time_pos++;
    }
    
    void handleEvents()
	{
        SDL_Event event;
        while (SDL_PollEvent(&event))
		{
            switch(event.type)
			{
                case SDL_QUIT:
                    isRunning=false;
                    break;
                    
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
					{
                        case SDLK_LEFT:move_left=true;break;
                        case SDLK_RIGHT:move_right=true;break;
                        case SDLK_UP:move_up=true;break;
                        case SDLK_DOWN:move_down=true;break;
                        case SDLK_z:move_z=true;break;
                        case SDLK_x:move_x=true;break;
                        case SDLK_LCTRL:move_ctrl=true;break;
                        case SDLK_LSHIFT:move_shift=true;break;
                        case SDLK_ESCAPE:move_esc=true;break;
                        case SDLK_q:isRunning=false;break;
                        case SDLK_r:  // 按R键重新加载图片
                            loadImages();
                            break;
                    }
                    break;
                    
                case SDL_KEYUP:
                    switch(event.key.keysym.sym)
					{
                        case SDLK_LEFT:move_left=false;break;
                        case SDLK_RIGHT:move_right=false;break;
                        case SDLK_UP:move_up=false;break;
                        case SDLK_DOWN: move_down=false;break;
                        case SDLK_z:move_z=false;break;
                        case SDLK_x:move_x=false;break;
                        case SDLK_LCTRL:move_ctrl=false;break;
                        case SDLK_LSHIFT:move_shift=false;break;
                        case SDLK_ESCAPE:move_esc=false;break;
                    }
                    break;
            }
        }
    }
    
    void render()
	{
		if(stage==0)
		{
			textures[1].render(renderer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);//1
			if(stageticks>=0 && stageticks<=44)//3
			{
				;
			}
			else if(stageticks>=45 && stageticks<=60)
			{
				textures[3].render(renderer,int(-60+sin((stageticks-45)/15.0/2*3.14)*120),int(400-sin((stageticks-45)/15.0/2*3.14)*390),256,31);
			}
			else
			{
				textures[3].render(renderer,60,10,256,31);
			}
			if(stageticks>=0 && stageticks<=30)//2
			{
				textures[2].render(renderer,(int)(394-stageticks/30.0*34),(int)(175-stageticks/30.0*165),
								   (int)(stageticks/30.0*34*2),(int)(stageticks/30.0*165*2));
			}
			else
			{
				textures[2].render(renderer,360,10,68,330);
			}
			textures[4].render(renderer,0,329,320,31);
				
		}
		else if(stage==1)
		{
			textures[1].render(renderer,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);//1
			if(stageticks>=-30 && stageticks<=-1)
			{
				textures[2].render(renderer,int(10-sin((stageticks+30)/60.0/2*3.14)*1000),10,68,330);
			}
			else
			{
				textures[3].render(renderer,60,10,256,31);
			}
			if(stageticks>=0 && stageticks<=60)//2
			{
				textures[2].render(renderer,int(394-sin(stageticks/60.0/2*3.14)*384),10,68,330);
			}
			else if(stageticks>=-30 && stageticks<=-1)
			{
				textures[2].render(renderer,int(10-sin((stageticks+30)/60.0/2*3.14)*1000),10,68,330);
			}
			else
			{
				textures[2].render(renderer,10,10,68,330);
			}
			textures[4].render(renderer,0,329,320,31);
			if(stageticks>=0 && stageticks<=60)//text
			{
				int tmp=int(500-sin(stageticks/60.0/2*3.14)*200);
				smallFont.drawString("Start",tmp,100,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Extra Start",tmp,124,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Practice Start",tmp,148,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Replay",tmp,172,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Score",tmp,196,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Music Room",tmp,220,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Option",tmp,244,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Quit",tmp,268,1.5,1.5,Color(0,0,0));
			}
			else if(stageticks>=-30 && stageticks<=-1)
			{
				int tmp=int(300-sin((stageticks+30)/60.0/2*3.14)*1000);
				smallFont.drawString("Start",tmp,100,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Extra Start",tmp,124,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Practice Start",tmp,148,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Replay",tmp,172,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Score",tmp,196,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Music Room",tmp,220,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Option",tmp,244,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Quit",tmp,268,1.5,1.5,Color(0,0,0));
			}
			else
			{
				smallFont.drawString("Start",300,100,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Extra Start",300,124,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Practice Start",300,148,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Replay",300,172,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Score",300,196,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Music Room",300,220,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Option",300,244,1.5,1.5,Color(0,0,0));
				smallFont.drawString("Quit",300,268,1.5,1.5,Color(0,0,0));
			}
			if(stageticks>=0 && stageticks<=60)//cursor
			{
				int tmp=int(500-sin(stageticks/60.0/2*3.14)*200);
				textures[5].render(renderer,tmp-20,(cursorpos[1]-1)*24+108,15,15);
			}
			else if(stageticks>=-30 && stageticks<=-1)
			{
				textures[2].render(renderer,int(280-sin((stageticks+30)/60.0/2*3.14)*1000),10,68,330);
			}
			else
			{
				textures[5].render(renderer,280,(cursorpos[1]-1)*24+108,15,15);
			}
		}
		else if(stage==2)
		{
			if(stageticks>=0 && stageticks<=30)
			{
				textures[6].setAlpha(int(stageticks/30.0*100));
			}
			textures[6].render(renderer,0,0,480,360);
			if(stageticks>=0 && stageticks<=30)
			{
				textures[7].render(renderer,146,int(-90+stageticks/30.0*100),189,40);
			}
			else if(stageticks>=-30 && stageticks<=-1)
			{
				textures[7].render(renderer,146,int(-90+(1-stageticks)/30.0*100),189,40);
			}
			else if(stageticks>=-150 && stageticks<=-120)
			{
				textures[7].render(renderer,146-int((-120-stageticks)/30.0*341),10,189,40);
			}
			else
			{
				textures[7].render(renderer,146,10,189,40);
			}
			for(int i=1;i<=4;i++)
			{
				if(cursorpos[2]==i)
				{
					textures[7+i].setAlpha(255);
				}
				else
				{
					textures[7+i].setAlpha(50);
				}
				if(stageticks>=0 && stageticks<=30)
				{
					textures[7+i].render(renderer,481-int(stageticks/30.0*340),75*i-10,198,66);
				}
				else if(stageticks>=-30 && stageticks<=-1)
				{
					textures[7+i].render(renderer,481-int((1-stageticks)/30.0*340),75*i-10,198,66);
				}
				else if(stageticks>=-150 && stageticks<=-120)
				{
					if(cursorpos[2]==i)
					{
						textures[7+i].render(renderer,141-int((-120-stageticks)/30.0*131),75*i-10+int((-120-stageticks)/30.0*((280)-(75*i-10))),198,66);
					}
					else
					{
						textures[7+i].render(renderer,141-int((-120-stageticks)/30.0*341),75*i-10,198,66);
					}
				}
				else
				{
					textures[7+i].render(renderer,141,75*i-10,198,66);
				}
				if(stageticks>=-150 && stageticks<=-120)
				{
					textures[12].render(renderer,100,10+(((-120-stageticks)-30)/30.0*100),189,40);
				}
			}
		}
		else if(stage==3)
		{
			textures[6].render(renderer,0,0,480,360);
			if(0<=stageticks && stageticks<=30)
			{
				for(int i=1;i<=4;i++)
				{
					if(cursorpos[2]==i)
					{
						textures[7+i].setAlpha(255-int(stageticks/30.0*155));
						textures[7+i].render(renderer,141-int(stageticks/30.0*131),75*i-10+int(stageticks/30.0*((280)-(75*i-10))),198,66);
					}
					else
					{
						textures[7+i].setAlpha(50);
						textures[7+i].render(renderer,141-int(stageticks/30.0*341),75*i-10,198,66);
					}
				}
				textures[7].render(renderer,146-int(stageticks/30.0*341),10,189,40);
			}
			else
			{
				textures[7+cursorpos[2]].setAlpha(100);
				textures[7+cursorpos[2]].render(renderer,10,280,198,66);
			}
			if(0<=stageticks && stageticks<=30)
			{
				textures[12].render(renderer,100,10+((stageticks-30)/30.0*100),189,40);
			}
			else
			{
				textures[12].render(renderer,100,10,189,40);
			}
			textures[13].setAlpha(100);
			textures[14].setAlpha(100);
			if(-100<=stageticks && stageticks<=-90)//r
			{
				if(cursorpos[3]==1)//s<-a
				{
					textures[14].render(renderer,280,0,int(160*(-90-stageticks)/10.0),360);
					textures[16].render(renderer,280,0,int(160*(-90-stageticks)/10.0),360);
					textures[18].render(renderer,280,0,int(160*(-90-stageticks)/10.0),360);
					textures[13].render(renderer,280+int(160*(-90-stageticks)/10.0),0,160-int(160*(-90-stageticks)/10.0),360);
					textures[15].render(renderer,280+int(160*(-90-stageticks)/10.0),0,160-int(160*(-90-stageticks)/10.0),360);
					textures[17].render(renderer,280+int(160*(-90-stageticks)/10.0),0,160-int(160*(-90-stageticks)/10.0),360);
				}
				else//a<-s
				{
					textures[13].render(renderer,280,0,int(160*(-90-stageticks)/10.0),360);
					textures[15].render(renderer,280,0,int(160*(-90-stageticks)/10.0),360);
					textures[17].render(renderer,280,0,int(160*(-90-stageticks)/10.0),360);
					textures[14].render(renderer,280+int(160*(-90-stageticks)/10.0),0,160-int(160*(-90-stageticks)/10.0),360);
					textures[16].render(renderer,280+int(160*(-90-stageticks)/10.0),0,160-int(160*(-90-stageticks)/10.0),360);
					textures[18].render(renderer,280+int(160*(-90-stageticks)/10.0),0,160-int(160*(-90-stageticks)/10.0),360);
				}
				
			}
			else if(-150<=stageticks && stageticks<=-140)//l
			{
				if(cursorpos[3]==1)//a->s
				{
					textures[13].render(renderer,280,0,int(160*(150+stageticks)/10.0),360);
					textures[15].render(renderer,280,0,int(160*(150+stageticks)/10.0),360);
					textures[17].render(renderer,280,0,int(160*(150+stageticks)/10.0),360);
					textures[14].render(renderer,280+int(160*(150+stageticks)/10.0),0,160-int(160*(150+stageticks)/10.0),360);
					textures[16].render(renderer,280+int(160*(150+stageticks)/10.0),0,160-int(160*(150+stageticks)/10.0),360);
					textures[18].render(renderer,280+int(160*(150+stageticks)/10.0),0,160-int(160*(150+stageticks)/10.0),360);
				}
				else//s->a
				{
					textures[14].render(renderer,280,0,int(160*(150+stageticks)/10.0),360);
					textures[16].render(renderer,280,0,int(160*(150+stageticks)/10.0),360);
					textures[18].render(renderer,280,0,int(160*(150+stageticks)/10.0),360);
					textures[13].render(renderer,280+int(160*(150+stageticks)/10.0),0,160-int(160*(150+stageticks)/10.0),360);
					textures[15].render(renderer,280+int(160*(150+stageticks)/10.0),0,160-int(160*(150+stageticks)/10.0),360);
					textures[17].render(renderer,280+int(160*(150+stageticks)/10.0),0,160-int(160*(150+stageticks)/10.0),360);
				}
			}
			else
			{
				if(cursorpos[3]==1)
				{
					textures[13].render(renderer,280,0,160,360);
					textures[15].render(renderer,280,0,160,360);
					textures[17].render(renderer,280,0,160,360);
				}
				else
				{
					textures[14].render(renderer,280,0,160,360);
					textures[16].render(renderer,280,0,160,360);
					textures[18].render(renderer,280,0,160,360);
				}
			}
		}
		else if(stage==4)
		{
			textures[6].render(renderer,0,0,480,360);
			if(0<=stageticks && stageticks<=30)
			{
				textures[7+cursorpos[2]].setAlpha(100);
				textures[7+cursorpos[2]].render(renderer,10,280,198,66);
				textures[7].setAlpha(100);
				textures[12].setAlpha(255-(stageticks/30.0*155));
				textures[13].setAlpha(100-(stageticks/30.0*50));
				textures[14].setAlpha(100-(stageticks/30.0*50));
				textures[15].setAlpha(255-(stageticks/30.0*155));
				textures[16].setAlpha(255-(stageticks/30.0*155));
				textures[17].setAlpha(255-(stageticks/30.0*155));
				textures[18].setAlpha(255-(stageticks/30.0*155));
				textures[12].render(renderer,100,10-((stageticks)/30.0*150),189,40);
				textures[12+cursorpos[3]].render(renderer,280-(stageticks/30.0*180),0,160,360);
				textures[14+cursorpos[3]].render(renderer,280-(stageticks/30.0*180),0,160,360);
				textures[16+cursorpos[3]].render(renderer,280-(stageticks/30.0*180),0,160,360);
				textures[19].render(renderer,200,-90+int((stageticks)/30.0*140),226,40);
				for(int i=1;i<=2;i++)
				{
					if(i==cursorpos[4])
					{
						textures[17+cursorpos[3]*2+i].setAlpha(255);
					}
					else
					{
						textures[17+cursorpos[3]*2+i].setAlpha(100);
					}
				}
				textures[17+cursorpos[3]*2+1].render(renderer,200+((30-stageticks)/30.0*280),100,226,39);
				textures[17+cursorpos[3]*2+2].render(renderer,200+((30-stageticks)/30.0*280),150,226,39);
			}
			else if(stageticks>=-50 && stageticks<=-20)
			{
				textures[7+cursorpos[2]].setAlpha(100);
				textures[7+cursorpos[2]].render(renderer,10,280,198,66);
				textures[7].setAlpha(100);
				textures[12].setAlpha(255-((-20-stageticks)/30.0*155));
				textures[13].setAlpha(100-((-20-stageticks)/30.0*50));
				textures[14].setAlpha(100-((-20-stageticks)/30.0*50));
				textures[15].setAlpha(255-((-20-stageticks)/30.0*155));
				textures[16].setAlpha(255-((-20-stageticks)/30.0*155));
				textures[17].setAlpha(255-((-20-stageticks)/30.0*155));
				textures[18].setAlpha(255-((-20-stageticks)/30.0*155));
				textures[12].render(renderer,100,10-(((-20-stageticks))/30.0*150),189,40);
				textures[12+cursorpos[3]].render(renderer,280-((-20-stageticks)/30.0*180),0,160,360);
				textures[14+cursorpos[3]].render(renderer,280-((-20-stageticks)/30.0*180),0,160,360);
				textures[16+cursorpos[3]].render(renderer,280-((-20-stageticks)/30.0*180),0,160,360);
				textures[19].render(renderer,200,-90+int(((-20-stageticks))/30.0*140),226,40);
				for(int i=1;i<=2;i++)
				{
					if(i==cursorpos[4])
					{
						textures[17+cursorpos[3]*2+i].setAlpha(255);
					}
					else
					{
						textures[17+cursorpos[3]*2+i].setAlpha(100);
					}
				}
				textures[17+cursorpos[3]*2+1].render(renderer,200+((30-(-20-stageticks))/30.0*280),100,226,39);
				textures[17+cursorpos[3]*2+2].render(renderer,200+((30-(-20-stageticks))/30.0*280),150,226,39);
			}
			else
			{
				textures[7+cursorpos[2]].setAlpha(100);
				textures[7+cursorpos[2]].render(renderer,10,280,198,66);
				textures[12+cursorpos[3]].render(renderer,100,0,160,360);
				textures[14+cursorpos[3]].render(renderer,100,0,160,360);
				textures[16+cursorpos[3]].render(renderer,100,0,160,360);
				textures[19].render(renderer,200,50,226,40);
				for(int i=1;i<=2;i++)
				{
					if(i==cursorpos[4])
					{
						textures[17+cursorpos[3]*2+i].setAlpha(255);
					}
					else
					{
						textures[17+cursorpos[3]*2+i].setAlpha(100);
					}
				}
				textures[17+cursorpos[3]*2+1].render(renderer,200,100,226,39);
				textures[17+cursorpos[3]*2+2].render(renderer,200,150,226,39);
			}
		}
		else if(stage==5)
		{
			textures[1].render(renderer,0,0,480,360);
			
			textures[29+(player_type+1)/2].render(renderer,player_x-player_size/2,player_y-player_size/2-8);
			textures[31+lv].render(renderer,player_x-player_size/2-8,player_y-player_size/2+8,8,8);
			textures[31+lv].render(renderer,player_x-player_size/2+8,player_y-player_size/2+8,8,8);
			
	        std::queue<drop> tmp;
	        while(!drops.empty())
	        {
	        	drop top=drops.front();
	        	drops.pop();
	        	textures[40+top.type].render(renderer,top.x-8,top.y-8,16,16);
	        	tmp.push(top);
			}
			while(!tmp.empty())
			{
				drops.push(tmp.front());
				tmp.pop();
			}
			
	        std::queue<bullet_good> tmp2;
	        while(!bullet_goods.empty())
	        {
	        	bullet_good top=bullet_goods.front();
	        	bullet_goods.pop();
	        	textures[40+top.type].render(renderer,top.x-8,top.y-8,16,16);
	        	tmp2.push(top);
			}
			while(!tmp2.empty())
			{
				bullet_goods.push(tmp2.front());
				tmp2.pop();
			}
			
	        std::queue<enemy> tmp3;
	        while(!enemys.empty())
	        {
	        	enemy top=enemys.front();
	        	enemys.pop();
	        	textures[31+top.type].render(renderer,top.x-8,top.y-8,16,16);
	        	tmp3.push(top);
			}
			while(!tmp3.empty())
			{
				enemys.push(tmp3.front());
				tmp3.pop();
			}
			textures[24].render(renderer,0,0,480,360);
			if(0<=stageticks && stageticks<120)
			{
				if(0<=stageticks && stageticks<60)
				{
					int ticktmp=stageticks/60.0*36;
					textures[25].render(renderer,375,260-ticktmp,40,40);
					textures[26].render(renderer,375,260-ticktmp,40,40);
					textures[27].render(renderer,375,260,40,40);
					textures[28].render(renderer,375-ticktmp,260,40,40);
					textures[29].render(renderer,375+ticktmp,260,40,40);
				}
				else if(60<=stageticks && stageticks<120)
				{
					int ticktmp=(stageticks-60)/60.0*36;
					textures[25].render(renderer,375,224-ticktmp,40,40);
					textures[26].render(renderer,375,224,40,40);
					textures[27].render(renderer,375,260,40,40);
					textures[28].render(renderer,339,260+ticktmp,40,40);
					textures[29].render(renderer,411,260+ticktmp,40,40);
				}
			}
			else
			{
				int ticktmp=(stageticks-60)/60.0*40;
				textures[25].render(renderer,375,188,40,40);
				textures[26].render(renderer,375,224,40,40);
				textures[27].render(renderer,375,260,40,40);
				textures[28].render(renderer,339,296,40,40);
				textures[29].render(renderer,411,296,40,40);
			}
			char chars[150];
			int_to_chars(chars,hiscore,10);
			smallFont.drawString(chars,400,36);
			int_to_chars(chars,score,10);
			smallFont.drawString(chars,400,52);
			int_to_chars(chars,hp,10);
			smallFont.drawString(chars,400,84);
			int_to_chars(chars,bomb,10);
			smallFont.drawString(chars,400,100);
			int_to_chars(chars,power,10);
			smallFont.drawString(chars,400,132);
			int_to_chars(chars,graze,10);
			smallFont.drawString(chars,400,148);
			int_to_chars(chars,point,10);
			smallFont.drawString(chars,400,164);
			
		}
		if(1)//fps 
		{
			if(delta_time_pos%30==0)
			{
				delta_time_pos=0;
				fps=30/delta_time;
				delta_time=0;
			}
			char fps_char[100];
			std::sprintf(fps_char,"%.2f fps",fps);
			smallFont.drawString(fps_char,319,329,2.0,2.0,Color(0,0,0));
			smallFont.drawString(fps_char,319,331,2.0,2.0,Color(0,0,0));
			smallFont.drawString(fps_char,321,329,2.0,2.0,Color(0,0,0));
			smallFont.drawString(fps_char,321,331,2.0,2.0,Color(0,0,0));
			smallFont.drawString(fps_char,320,330,2.0,2.0,Color(255,255,255));
		}
		stageticks++;
	    SDL_RenderPresent(renderer);
		
    }
    
    void run()
	{
		if(shouldFullscreen)
		{
			toggleFullscreen();
		}
        while(isRunning)
		{
            Uint32 currentTime=SDL_GetTicks();
            float deltaTime=(currentTime-lastFrameTime)/1000.0f;
            if(deltaTime>0.033f)
			{
				deltaTime = 0.033f;
			}
            lastFrameTime=currentTime;
            
            handleEvents();
            update(deltaTime);
            render();
            
            int frameTime=SDL_GetTicks()-currentTime;
            if(frameTime<16)
			{
                SDL_Delay(16 - frameTime);
            }
        }
        
        Mix_HaltChannel(-1);
    }
};

#ifdef _WIN32
int SDL_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    ConsoleGame game;
    
    if (!game.init()) {
        std::cerr << "游戏初始化失败！" << std::endl;
        std::cerr << "请确保SDL2.dll和SDL2_mixer.dll在程序目录" << std::endl;
        system("pause");
        return -1;
    }
    
    game.run();
    
    return 0;
}
