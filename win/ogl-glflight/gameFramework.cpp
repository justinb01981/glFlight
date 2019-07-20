#include "gameFramework.hpp"
#include "png.hpp"
#include <glm/vector_relational.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gli/generate_mipmaps.hpp>
#include <gli/copy.hpp>
#include <gli/duplicate.hpp>
#include <fstream>


gameFramework::gameFramework
(
	int argc, char* argv[], char const* Title,
	profile Profile, int Major, int Minor,
	std::size_t FrameCount,
	success Success,
	glm::uvec2 const & WindowSize
) :
	gameFramework(argc, argv, Title, Profile, Major, Minor, WindowSize, glm::vec2(0), glm::vec2(0), FrameCount, Success)
{}

gameFramework::gameFramework
(
	int argc, char* argv[], char const* Title,
	profile Profile, int Major, int Minor,
	glm::vec2 const & Orientation,
	success Success
) :
	gameFramework(argc, argv, Title, Profile, Major, Minor, glm::uvec2(640, 480), Orientation, glm::vec2(0, 4), 2, Success)
{}

gameFramework::gameFramework
(
	int argc, char* argv[], char const* Title,
	profile Profile, int Major, int Minor,
	std::size_t FrameCount,
	glm::uvec2 const & WindowSize,
	glm::vec2 const & Orientation,
	glm::vec2 const & Position
) :
	gameFramework(argc, argv, Title, Profile, Major, Minor, WindowSize, Orientation, Position, FrameCount, RUN_ONLY)
{}

gameFramework::gameFramework
(
	int argc, char* argv[], char const* Title,
	profile Profile, int Major, int Minor,
	heuristic Heuristic
) :
	gameFramework(argc, argv, Title, Profile, Major, Minor, glm::uvec2(640, 480), glm::vec2(0), glm::vec2(0, 4), 2, MATCH_TEMPLATE, Heuristic)
{}

gameFramework::gameFramework
(
	int argc, char* argv[], char const* Title,
	profile Profile, int Major, int Minor,
	glm::uvec2 const & WindowSize, glm::vec2 const & Orientation, glm::vec2 const & Position,
	std::size_t FrameCount, success Success, heuristic Heuristic
) :
	Window(nullptr),
	Success(Success),
	Title(Title),
	Profile(Profile),
	Major(Major),
	Minor(Minor),
	TimerQueryName(0),
	FrameCount(FrameCount),
	TimeSum(0.0),
	TimeMin(std::numeric_limits<double>::max()),
	TimeMax(0.0),
	MouseOrigin(WindowSize >> 1u),
	MouseCurrent(WindowSize >> 1u),
	TranlationOrigin(Position),
	TranlationCurrent(Position),
	RotationOrigin(Orientation), 
	RotationCurrent(Orientation),
	MouseButtonFlags(0),
	Error(false),
	Heuristic(Heuristic),
	ViewSetupFlags(VIEW_SETUP_TRANSLATE | VIEW_SETUP_ROTATE_X | VIEW_SETUP_ROTATE_Y)
{
	assert(WindowSize.x > 0 && WindowSize.y > 0);

	memset(&KeyPressed[0], 0, sizeof(KeyPressed));

	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GL_FALSE);
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, Profile == ES ? GLFW_OPENGL_ES_API : GLFW_OPENGL_API);

	if(version(this->Major, this->Minor) >= version(3, 2) || (Profile == ES))
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, this->Major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, this->Minor);
#		if defined(__APPLE__)
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#		else
			if(Profile != ES)
			{
				glfwWindowHint(GLFW_OPENGL_PROFILE, Profile == CORE ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);
				glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, Profile == CORE ? GL_TRUE : GL_FALSE);
			}

#			if defined(_DEBUG)
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#			else
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
#			endif
#		endif
	}

#	if defined(__APPLE__)
		int const DPI = 2;
#	else
		int const DPI = 1;
#	endif
	
	this->Window = glfwCreateWindow(WindowSize.x / DPI, WindowSize.y / DPI, argv[0], nullptr, nullptr);

	if(this->Window)
	{
		glfwSetWindowPos(this->Window, 64, 64);
		glfwSetWindowUserPointer(this->Window, this);
		glfwSetMouseButtonCallback(this->Window, gameFramework::mouseButtonCallback);
		glfwSetCursorPosCallback(this->Window, gameFramework::cursorPositionCallback);
		glfwSetKeyCallback(this->Window, gameFramework::keyCallback);
		glfwMakeContextCurrent(this->Window);

		glewExperimental = GL_TRUE;
		glewInit();
		glGetError();

#		if defined(_DEBUG) && defined(GL_KHR_debug)
			if(this->isExtensionSupported("GL_KHR_debug"))
			{
				glEnable(GL_DEBUG_OUTPUT);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
				glDebugMessageCallback(&gameFramework::debugOutput, this);
			}
#		endif

		glGenQueries(1, &this->TimerQueryName);
	}
}

gameFramework::~gameFramework()
{
	if(this->TimerQueryName)
		glDeleteQueries(1, &this->TimerQueryName);

	if(this->Window)
	{
		glfwDestroyWindow(this->Window);
		this->Window = 0;
	}

	glfwTerminate();
}

int gameFramework::operator()()
{
	if(this->Window == 0)
		return EXIT_FAILURE;

	int Result = EXIT_SUCCESS;
	
	if(Result == EXIT_SUCCESS)
		if(version(this->Major, this->Minor) >= version(3, 0))
			Result = checkGLVersion(this->Major, this->Minor) ? EXIT_SUCCESS : EXIT_FAILURE;

	if(Result == EXIT_SUCCESS)
		Result = this->begin() ? EXIT_SUCCESS : EXIT_FAILURE;

	std::size_t FrameNum = 0;
	bool Automated = false;
#	ifdef AUTOMATED_TESTS
		Automated = true;
		FrameNum = this->FrameCount;
#	endif//AUTOMATED_TESTS

	while(Result == EXIT_SUCCESS && !this->Error)
	{
		Result = this->render() ? EXIT_SUCCESS : EXIT_FAILURE;
		Result = Result && this->checkError("render");

		glfwPollEvents();
		if(glfwWindowShouldClose(this->Window) || (Automated && FrameNum == 0))
		{
			if(this->Success == MATCH_TEMPLATE)
			{
				if(!checkTemplate(this->Window, this->Title.c_str()))
					Result = EXIT_FAILURE;
				this->checkError("checkTemplate");
			}
			break;
		}

		this->swap();

		if(Automated)
			--FrameNum;
	}

	if (Result == EXIT_SUCCESS)
		Result = this->end() && (Result == EXIT_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;

	if(this->Success == GENERATE_ERROR)
		return (Result != EXIT_SUCCESS || this->Error) ? EXIT_SUCCESS : EXIT_FAILURE;
	else
		return (Result == EXIT_SUCCESS && !this->Error) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void gameFramework::swap()
{
	glfwSwapBuffers(this->Window);
}

void gameFramework::sync(sync_mode const & Sync)
{
	switch(Sync)
	{
	case ASYNC:
		glfwSwapInterval(0);
		break;
	case VSYNC:
		glfwSwapInterval(1);
		break;
	case TEARING:
		glfwSwapInterval(-1);
		break;
	default:
		assert(0);
	}
}

void gameFramework::stop()
{
	glfwSetWindowShouldClose(this->Window, GL_TRUE);
}

void gameFramework::log(csv & CSV, char const* String)
{
	CSV.log(String, this->TimeSum / this->FrameCount, this->TimeMin, this->TimeMax);
}

void gameFramework::setupView(bool Translate, bool RotateX, bool RotateY)
{
	this->ViewSetupFlags =
		(Translate ? VIEW_SETUP_TRANSLATE : 0) |
		(RotateX ? VIEW_SETUP_ROTATE_X : 0) |
		(RotateY ? VIEW_SETUP_ROTATE_Y : 0);
}

bool gameFramework::isExtensionSupported(char const* String)
{
	GLint ExtensionCount(0);
	glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);
	for(GLint i = 0; i < ExtensionCount; ++i)
		if(std::string((char const*)glGetStringi(GL_EXTENSIONS, i)) == std::string(String))
			return true;
	//printf("Failed to find Extension: \"%s\"\n",String);
	return false;
}

glm::uvec2 gameFramework::getWindowSize() const
{
	glm::ivec2 WindowSize(0);
	glfwGetFramebufferSize(this->Window, &WindowSize.x, &WindowSize.y);
	return glm::uvec2(WindowSize);
}

bool gameFramework::isKeyPressed(int Key) const
{
	return this->KeyPressed[Key];
}

glm::mat4 gameFramework::view() const
{
	glm::mat4 ViewTranslate(1.0f);
	if(this->ViewSetupFlags & VIEW_SETUP_TRANSLATE)
		ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -this->TranlationCurrent.y));

	glm::mat4 ViewRotateX = ViewTranslate;
	if(this->ViewSetupFlags & VIEW_SETUP_ROTATE_X)
		ViewRotateX = glm::rotate(ViewTranslate, this->RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));

	glm::mat4 View = ViewRotateX;
	if(this->ViewSetupFlags & VIEW_SETUP_ROTATE_Y)
		View = glm::rotate(ViewRotateX, this->RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
	return View;
}

glm::vec3 gameFramework::cameraPosition() const
{
	return glm::vec3(0.0f, 0.0f, -this->TranlationCurrent.y);
}

namespace
{
	gli::texture absolute_difference(gli::texture const& A, gli::texture const& B, glm::u8 Scale)
	{
		assert(A.format() == gli::FORMAT_RGB8_UNORM_PACK8 && B.format() == gli::FORMAT_RGB8_UNORM_PACK8);

		gli::texture Result(A.target(), A.format(), A.extent(), A.layers(), A.faces(), A.levels());
		for(std::size_t TexelIndex = 0, TexelCount = A.size<glm::u8vec3>(); TexelIndex < TexelCount; ++TexelIndex)
		{
			glm::u8vec3 const TexelA = *(A.data<glm::u8vec3>() + TexelIndex);
			glm::u8vec3 const TexelB = *(B.data<glm::u8vec3>() + TexelIndex);
			glm::u8vec3 const TexelResult = glm::mix(TexelA - TexelB, TexelB - TexelA, glm::greaterThan(TexelB, TexelA)) * glm::u8vec3(Scale);
			*(Result.data<glm::u8vec3>() + TexelIndex) = TexelResult;
		}
		return Result;
	}

	struct heuristic
	{
		virtual bool test(gli::texture const& A, gli::texture const& B) const = 0;
	};

	struct heuristic_equal : public heuristic
	{
		bool test(gli::texture const& A, gli::texture const& B) const override
		{
			return A == B;
		}
	};

	struct heuristic_absolute_difference_max_one_large_kernel
	{
		bool kernel(gli::texture2d::extent_type const& TexelCoordA, glm::u8vec3 const& TexelA, gli::texture2d const& TextureB) const
		{
			int const KernelSize = 9;

			gli::texture2d::extent_type const TexelCoordInvA(TexelCoordA.x, 480 - TexelCoordA.y);
			glm::u8vec3 TexelB[KernelSize * KernelSize];

			for(int KernelIndexY = 0; KernelIndexY < KernelSize; ++KernelIndexY)
			for(int KernelIndexX = 0; KernelIndexX < KernelSize; ++KernelIndexX)
			{
				gli::texture2d::extent_type const KernelCoordB(KernelIndexX - KernelSize / 2, KernelIndexY - KernelSize / 2);
				gli::texture2d::extent_type const TexelCoordB = TexelCoordA + KernelCoordB;

				gli::texture2d::extent_type ClampedTexelCoord = glm::clamp(TexelCoordB, glm::ivec2(0), glm::ivec2(TextureB.extent()) - glm::ivec2(1));
				TexelB[KernelIndexY * KernelSize + KernelIndexX] = TextureB.load<glm::u8vec3>(ClampedTexelCoord, 0);
			}

			for(int KernelIndex = 0; KernelIndex < KernelSize * KernelSize; ++KernelIndex)
			{
				glm::vec3 const TexelDiff = glm::abs(glm::vec3(TexelB[KernelIndex]) - glm::vec3(TexelA));
				if(glm::all(glm::lessThanEqual(TexelDiff, glm::vec3(2))))
					return true;
				continue;
			}

			return false;
		}

		bool test(gli::texture const& A, gli::texture const& B) const
		{
			gli::texture2d TextureA(A);
			gli::texture2d TextureB(B);

			for(std::size_t TexelIndexY = 0, TexelCountY = A.extent().y; TexelIndexY < TexelCountY; ++TexelIndexY)
			for(std::size_t TexelIndexX = 0, TexelCountX = A.extent().x; TexelIndexX < TexelCountX; ++TexelIndexX)
			{
				gli::texture2d::extent_type const TexelCoordA(TexelIndexX, TexelIndexY);
				glm::u8vec3 const TexelA = TextureA.load<glm::u8vec3>(TexelCoordA, 0);
				glm::u8vec3 const TexelB = TextureB.load<glm::u8vec3>(TexelCoordA, 0);
				if (TexelA == TexelB)
					continue;

				bool const ValidTexel = this->kernel(TexelCoordA, TexelA, TextureB);
				if(!ValidTexel)
					return false;
			}

			return true;
		}
	};

	struct heuristic_absolute_difference_max_one_kernel
	{
		bool test(gli::texture const& A, gli::texture const& B) const
		{
			gli::texture2d Texture(absolute_difference(A, B, 1));
			glm::u8vec3 AbsDiffMax(0);
			for(std::size_t TexelIndexY = 0, TexelCountY = Texture.extent().y; TexelIndexY < TexelCountY; ++TexelIndexY)
			for(std::size_t TexelIndexX = 0, TexelCountX = Texture.extent().x; TexelIndexX < TexelCountX; ++TexelIndexX)
			{
				gli::texture2d::extent_type const TexelCoord(TexelIndexX, TexelIndexY);
				glm::u8vec3 TexelDiff = Texture.load<glm::u8vec3>(TexelCoord, 0);

				if(glm::all(glm::lessThanEqual(TexelDiff, glm::u8vec3(1))))
					continue;

				gli::texture2d TextureA(A);
				gli::texture2d TextureB(B);
				glm::u8vec3 const TexelA = TextureA.load<glm::u8vec3>(TexelCoord, 0);

				bool KernelAbsDiffMax = false;
				for(int KernelIndexY = -1; KernelIndexY <= 1; ++KernelIndexY)
				for(int KernelIndexX = -1; KernelIndexX <= 1; ++KernelIndexX)
				{
					glm::ivec2 const KernelCoord(KernelIndexX, KernelIndexY);
					gli::texture2d::extent_type ClampedTexelCoord = glm::clamp(glm::ivec2(TexelCoord) + KernelCoord, glm::ivec2(0), glm::ivec2(Texture.extent()) - glm::ivec2(1));
					glm::u8vec3 const TexelB = TextureB.load<glm::u8vec3>(ClampedTexelCoord, 0);

					if(glm::all(glm::lessThanEqual(glm::abs(glm::vec3(TexelB) - glm::vec3(TexelA)), glm::vec3(1))))
						KernelAbsDiffMax = true;
				}

				if(KernelAbsDiffMax)
					TexelDiff = glm::min(TexelDiff, glm::u8vec3(1));
				AbsDiffMax = glm::max(TexelDiff, AbsDiffMax);
			}

			return glm::all(glm::lessThanEqual(AbsDiffMax, glm::u8vec3(1)));
		}
	};

	struct heuristic_absolute_difference_max_one
	{
		bool test(gli::texture const& A, gli::texture const& B) const
		{
			gli::texture Texture = absolute_difference(A, B, 1);
			glm::u8vec3 AbsDiffMax(0);
			glm::u32vec3 AbsDiffCount(0);
			for(std::size_t TexelIndex = 0, TexelCount = Texture.size<glm::u8vec3>(); TexelIndex < TexelCount; ++TexelIndex)
			{
				glm::u8vec3 AbsDiff = *(Texture.data<glm::u8vec3>() + TexelIndex);
				if(AbsDiff.x > 0)
					++AbsDiffCount.x;
				if(AbsDiff.y > 0)
					++AbsDiffCount.y;
				if(AbsDiff.z > 0)
					++AbsDiffCount.z;
				AbsDiffMax = glm::max(AbsDiff, AbsDiffMax);
			}
			return glm::all(glm::lessThanEqual(AbsDiffMax, glm::u8vec3(1)));
		}
	};

	struct heuristic_mipmaps_absolute_difference_max_one
	{
		bool test(gli::texture const& A, gli::texture const& B) const
		{
			gli::texture2d TextureA(A);
			gli::texture2d TextureB(B);
			gli::texture2d MipmapsA(TextureA.format(), TextureA.extent());
			gli::texture2d MipmapsB(TextureB.format(), TextureB.extent());
			memcpy(MipmapsA.data(), TextureA.data(), TextureA.size());
			memcpy(MipmapsB.data(), TextureB.data(), TextureB.size());
			gli::texture2d GeneratedA = gli::generate_mipmaps(MipmapsA, gli::FILTER_LINEAR);
			gli::texture2d GeneratedB = gli::generate_mipmaps(MipmapsB, gli::FILTER_LINEAR);
			gli::texture ViewA = gli::view(GeneratedA, 3, 3);
			gli::texture ViewB = gli::view(GeneratedB, 3, 3);
			gli::texture Texture = absolute_difference(ViewA, ViewB, 1);
			glm::u8vec3 AbsDiffMax(0);
			glm::u32vec3 AbsDiffCount(0);
			for(std::size_t TexelIndex = 0, TexelCount = Texture.size<glm::u8vec3>(); TexelIndex < TexelCount; ++TexelIndex)
			{
				glm::u8vec3 AbsDiff = *(Texture.data<glm::u8vec3>() + TexelIndex);
				if(AbsDiff.x > 0)
					++AbsDiffCount.x;
				if(AbsDiff.y > 0)
					++AbsDiffCount.y;
				if(AbsDiff.z > 0)
					++AbsDiffCount.z;
				AbsDiffMax = glm::max(AbsDiff, AbsDiffMax);
			}
			return glm::all(glm::lessThanEqual(AbsDiffMax, glm::u8vec3(1)));
		}
	};

	struct heuristic_mipmaps_absolute_difference_max_four
	{
		bool test(gli::texture const& A, gli::texture const& B) const
		{
			gli::texture2d TextureA(A);
			gli::texture2d TextureB(B);
			gli::texture2d MipmapsA(TextureA.format(), TextureA.extent());
			gli::texture2d MipmapsB(TextureB.format(), TextureB.extent());
			memcpy(MipmapsA.data(), TextureA.data(), TextureA.size());
			memcpy(MipmapsB.data(), TextureB.data(), TextureB.size());
			gli::texture2d GeneratedA = gli::generate_mipmaps(MipmapsA, gli::FILTER_LINEAR);
			gli::texture2d GeneratedB = gli::generate_mipmaps(MipmapsB, gli::FILTER_LINEAR);
			gli::texture ViewA = gli::view(GeneratedA, 3, 3);
			gli::texture ViewB = gli::view(GeneratedB, 3, 3);
			gli::texture Texture = absolute_difference(ViewA, ViewB, 1);
			glm::u8vec3 AbsDiffMax(0);
			glm::u32vec3 AbsDiffCount(0);
			for(std::size_t TexelIndex = 0, TexelCount = Texture.size<glm::u8vec3>(); TexelIndex < TexelCount; ++TexelIndex)
			{
				glm::u8vec3 AbsDiff = *(Texture.data<glm::u8vec3>() + TexelIndex);
				if(AbsDiff.x > 0)
					++AbsDiffCount.x;
				if(AbsDiff.y > 0)
					++AbsDiffCount.y;
				if(AbsDiff.z > 0)
					++AbsDiffCount.z;
				AbsDiffMax = glm::max(AbsDiff, AbsDiffMax);
			}
			return glm::all(glm::lessThanEqual(AbsDiffMax, glm::u8vec3(4)));
		}
	};

	struct heuristic_mipmaps_absolute_difference_max_channel
	{
		bool kernel(gli::texture2d::extent_type const& TexelCoordA, glm::u8vec3 const& TexelA, gli::texture2d const& TextureB) const
		{
			int const KernelSize = 1;

			glm::u8vec3 TexelB[KernelSize * KernelSize];

			for(int KernelIndexY = 0; KernelIndexY < KernelSize; ++KernelIndexY)
			for(int KernelIndexX = 0; KernelIndexX < KernelSize; ++KernelIndexX)
			{
				gli::texture2d::extent_type const KernelCoordB(KernelIndexX - KernelSize / 2, KernelIndexY - KernelSize / 2);
				gli::texture2d::extent_type const TexelCoordB = TexelCoordA + KernelCoordB;

				gli::texture2d::extent_type ClampedTexelCoord = glm::clamp(TexelCoordB, glm::ivec2(0), glm::ivec2(TextureB.extent()) - glm::ivec2(1));
				TexelB[KernelIndexY * KernelSize + KernelIndexX] = TextureB.load<glm::u8vec3>(ClampedTexelCoord, 0);
			}

			glm::vec3 TexelDiff[KernelSize * KernelSize];
			for(int KernelIndex = 0; KernelIndex < KernelSize * KernelSize; ++KernelIndex)
			{
				TexelDiff[KernelIndex] = glm::abs(glm::vec3(TexelB[KernelIndex]) - glm::vec3(TexelA));

				float MaxComponent = glm::max(glm::max(TexelDiff[KernelIndex].r, TexelDiff[KernelIndex].g), TexelDiff[KernelIndex].b);
				float MinComponent = glm::min(glm::min(TexelDiff[KernelIndex].r, TexelDiff[KernelIndex].g), TexelDiff[KernelIndex].b);
				if(MaxComponent < 17.f && MinComponent < 6.f)
					return true;
				continue;
			}

			return false;
		}

		bool test(gli::texture const& A, gli::texture const& B) const
		{
			gli::texture2d TextureA(A);
			gli::texture2d TextureB(B);
			gli::texture2d MipmapsA(TextureA.format(), TextureA.extent());
			gli::texture2d MipmapsB(TextureB.format(), TextureB.extent());
			memcpy(MipmapsA.data(), TextureA.data(), TextureA.size());
			memcpy(MipmapsB.data(), TextureB.data(), TextureB.size());
			gli::texture2d GeneratedA = gli::generate_mipmaps(MipmapsA, gli::FILTER_LINEAR);
			gli::texture2d GeneratedB = gli::generate_mipmaps(MipmapsB, gli::FILTER_LINEAR);
			gli::texture2d ViewA(gli::view(GeneratedA, 3, 3));
			gli::texture2d ViewB(gli::view(GeneratedB, 3, 3));

			for(std::size_t TexelIndexY = 0, TexelCountY = ViewA.extent().y; TexelIndexY < TexelCountY; ++TexelIndexY)
			for(std::size_t TexelIndexX = 0, TexelCountX = ViewA.extent().x; TexelIndexX < TexelCountX; ++TexelIndexX)
			{
				gli::texture2d::extent_type const TexelCoordA(TexelIndexX, TexelIndexY);
				glm::u8vec3 const TexelA = ViewA.load<glm::u8vec3>(TexelCoordA, 0);
				glm::u8vec3 const TexelB = ViewB.load<glm::u8vec3>(TexelCoordA, 0);
				if (TexelA == TexelB)
					continue;

				bool const ValidTexel = this->kernel(TexelCoordA, TexelA, ViewB);
				if(!ValidTexel)
					return false;
			}

			return true;
		}
	};

	template <typename heuristic>
	bool compare(gli::texture const& A, gli::texture const& B, heuristic const& Euristic)
	{
		return Euristic.test(A, B);
	}
}//namespace

bool gameFramework::checkTemplate(GLFWwindow* pWindow, char const* Title)
{
    /*
	GLint ColorType = GL_UNSIGNED_BYTE;
	GLint ColorFormat = GL_RGBA;
		
	if (Profile == ES)
	{
		glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &ColorType);
		glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &ColorFormat);
	}

	GLint WindowSizeX(0);
	GLint WindowSizeY(0);
	glfwGetFramebufferSize(pWindow, &WindowSizeX, &WindowSizeY);

	gli::texture2d TextureRead(ColorFormat == GL_RGBA ? gli::FORMAT_RGBA8_UNORM_PACK8 : gli::FORMAT_RGB8_UNORM_PACK8, gli::texture2d::extent_type(WindowSizeX, WindowSizeY), 1);
	gli::texture2d TextureRGB(gli::FORMAT_RGB8_UNORM_PACK8, gli::texture2d::extent_type(WindowSizeX, WindowSizeY), 1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glReadPixels(0, 0, WindowSizeX, WindowSizeY, ColorFormat, ColorType, TextureRead.format() == gli::FORMAT_RGBA8_UNORM_PACK8 ? TextureRead.data() : TextureRGB.data());

	if(TextureRead.format() == gli::FORMAT_RGBA8_UNORM_PACK8)
	{
		for(gli::size_t y = 0; y < TextureRGB.extent().y; ++y)
		for(gli::size_t x = 0; x < TextureRGB.extent().x; ++x)
		{
			gli::texture2d::extent_type TexelCoord(x, y);

			glm::u8vec3 Color(TextureRead.load<glm::u8vec4>(TexelCoord, 0));
			TextureRGB.store(TexelCoord, 0, Color);
		}
	}

	bool Success = true;

	if(Success)
	{
		gli::texture Template(load_png((getDataDirectory() + "templates/" + Title + ".png").c_str()));

		if(Success)
			Success = Success && !Template.empty();

		bool SameSize = false;
		if(Success)
		{
			SameSize = gli::texture2d(Template).extent() == TextureRGB.extent();
			Success = Success && SameSize;
		}

		if(Success)
		{
			bool Pass = false;
			if(!Pass && this->Heuristic & HEURISTIC_EQUAL_BIT)
				Pass = compare(Template, TextureRGB, heuristic_equal());
			if(!Pass && (this->Heuristic & HEURISTIC_ABSOLUTE_DIFFERENCE_MAX_ONE_BIT))
				Pass = compare(Template, TextureRGB, heuristic_absolute_difference_max_one());
			if(!Pass && (this->Heuristic & HEURISTIC_ABSOLUTE_DIFFERENCE_MAX_ONE_KERNEL_BIT))
				Pass = compare(Template, TextureRGB, heuristic_absolute_difference_max_one_kernel());
			if(!Pass && (this->Heuristic & HEURISTIC_ABSOLUTE_DIFFERENCE_MAX_ONE_LARGE_KERNEL_BIT))
				Pass = compare(Template, TextureRGB, heuristic_absolute_difference_max_one_large_kernel());
			if(!Pass && (this->Heuristic & HEURISTIC_MIPMAPS_ABSOLUTE_DIFFERENCE_MAX_ONE_BIT))
				Pass = compare(Template, TextureRGB, heuristic_mipmaps_absolute_difference_max_one());
			if(!Pass && (this->Heuristic & HEURISTIC_MIPMAPS_ABSOLUTE_DIFFERENCE_MAX_FOUR_BIT))
				Pass = compare(Template, TextureRGB, heuristic_mipmaps_absolute_difference_max_four());
			if(!Pass && (this->Heuristic & HEURISTIC_MIPMAPS_ABSOLUTE_DIFFERENCE_MAX_CHANNEL_BIT))
				Pass = compare(Template, TextureRGB, heuristic_mipmaps_absolute_difference_max_channel());
			Success = Pass;
		}

		// Save abs diff
		if(!Success)
		{
			if(SameSize && !Template.empty())
			{
				gli::texture Diff = ::absolute_difference(Template, TextureRGB, 2);
				save_png(gli::texture2d(Diff), (getBinaryDirectory() + "/" + Title + "-diff.png").c_str());
			}

			if(!Template.empty())
				save_png(Template, (getBinaryDirectory() + "/" + Title + "-correct.png").c_str());

			save_png(TextureRGB, (getBinaryDirectory() + "/" + Title + ".png").c_str());
		}
	}
    */
	return Success;
}

void gameFramework::pollJoystick(float** axes, size_t *axes_n, unsigned char** buttons, size_t *buttons_n)
{
    int tmp1, tmp2;
    *axes = (float*) glfwGetJoystickAxes(GLFW_JOYSTICK_1, &tmp1);
    *axes_n = tmp1;

    *buttons = (unsigned char*) glfwGetJoystickButtons(GLFW_JOYSTICK_1, &tmp2);
    *buttons_n = tmp2;
}

void gameFramework::beginTimer()
{
	glBeginQuery(GL_TIME_ELAPSED, this->TimerQueryName);
}

void gameFramework::endTimer()
{
	glEndQuery(GL_TIME_ELAPSED);

	GLuint QueryTime(0);
	glGetQueryObjectuiv(this->TimerQueryName, GL_QUERY_RESULT, &QueryTime);

	double const InstantTime(static_cast<double>(QueryTime) / 1000.0);

	this->TimeSum += InstantTime;
	this->TimeMax = glm::max(this->TimeMax, InstantTime);
	this->TimeMin = glm::min(this->TimeMin, InstantTime);

	fprintf(stdout, "\rTime: %2.4f ms    ", InstantTime / 1000.0);
}

std::string gameFramework::loadFile(std::string const & Filename) const
{
	std::string Result;

	std::ifstream Stream(Filename.c_str());
	if(!Stream.is_open())
		return Result;

	Stream.seekg(0, std::ios::end);
	Result.reserve(Stream.tellg());
	Stream.seekg(0, std::ios::beg);

	Result.assign(
		(std::istreambuf_iterator<char>(Stream)),
		std::istreambuf_iterator<char>());

	return Result;
}

void gameFramework::logImplementationDependentLimit(GLenum Value, std::string const & String) const
{
	GLint Result(0);
	glGetIntegerv(Value, &Result);
	std::string Message(format("%s: %d", String.c_str(), Result));
#	if(!defined(__APPLE__) && defined(GL_ARB_debug_output))
		glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 1, GL_DEBUG_SEVERITY_LOW_ARB, GLsizei(Message.size()), Message.c_str());
#	endif
}

bool gameFramework::validate(GLuint VertexArrayName, std::vector<vertexattrib> const & Expected) const
{
	bool Success = true;
#if !defined(__APPLE__)
	GLint MaxVertexAttrib(0);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &MaxVertexAttrib);

	glBindVertexArray(VertexArrayName);
	for (GLuint AttribLocation = 0; AttribLocation < glm::min(GLuint(MaxVertexAttrib), GLuint(Expected.size())); ++AttribLocation)
	{
		vertexattrib VertexAttrib;
		glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &VertexAttrib.Enabled);
		glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &VertexAttrib.Binding);
		glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_SIZE, &VertexAttrib.Size);
		glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &VertexAttrib.Stride);
		glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_TYPE, &VertexAttrib.Type);
		glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &VertexAttrib.Normalized);
		if (this->Profile != ES || (this->Profile == ES && (this->Major * 10 + this->Minor >= 30)))
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_INTEGER, &VertexAttrib.Integer);
		if (this->Profile != ES && (this->Major * 10 + this->Minor >= 44))
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_LONG, &VertexAttrib.Long);
		if (this->Profile != ES && (this->Major * 10 + this->Minor >= 31))
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_DIVISOR, &VertexAttrib.Divisor);
		glGetVertexAttribPointerv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_POINTER, &VertexAttrib.Pointer);
		Success = Success && (VertexAttrib == Expected[AttribLocation]);
		assert(Success);
	}
	glBindVertexArray(0);
#endif//!defined(__APPLE__)
	return Success;
}

bool gameFramework::checkError(const char* Title) const
{
	int Error;
	if((Error = glGetError()) != GL_NO_ERROR)
	{
		std::string ErrorString;
		switch(Error)
		{
		case GL_INVALID_ENUM:
			ErrorString = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			ErrorString = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			ErrorString = "GL_INVALID_OPERATION";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			ErrorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			ErrorString = "GL_OUT_OF_MEMORY";
			break;
		default:
			ErrorString = "UNKNOWN";
			break;
		}
		fprintf(stdout, "OpenGL Error(%s): %s\n", ErrorString.c_str(), Title);
		assert(0);
	}
	return Error == GL_NO_ERROR;
}

bool gameFramework::checkFramebuffer(GLuint FramebufferName) const
{
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch(Status)
	{
	case GL_FRAMEBUFFER_UNDEFINED:
		fprintf(stdout, "OpenGL Error(%s)\n", "GL_FRAMEBUFFER_UNDEFINED");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		fprintf(stdout, "OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		fprintf(stdout, "OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		fprintf(stdout, "OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		fprintf(stdout, "OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		fprintf(stdout, "OpenGL Error(%s)\n", "GL_FRAMEBUFFER_UNSUPPORTED");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		fprintf(stdout, "OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		fprintf(stdout, "OpenGL Error(%s)\n", "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
		break;
	}

	return Status == GL_FRAMEBUFFER_COMPLETE;
}

bool gameFramework::checkExtension(char const* ExtensionName) const
{
	GLint ExtensionCount = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);
	for(GLint i = 0; i < ExtensionCount; ++i)
		if(std::string((char const*)glGetStringi(GL_EXTENSIONS, i)) == std::string(ExtensionName))
			return true;
	printf("Failed to find Extension: \"%s\"\n", ExtensionName);
	return false;
}

bool gameFramework::checkGLVersion(GLint MajorVersionRequire, GLint MinorVersionRequire) const
{
	GLint MajorVersionContext = 0;
	GLint MinorVersionContext = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &MajorVersionContext);
	glGetIntegerv(GL_MINOR_VERSION, &MinorVersionContext);
	printf("OpenGL Version Needed %d.%d ( %d.%d Found )\n",
		MajorVersionRequire, MinorVersionRequire,
		MajorVersionContext, MinorVersionContext);
	return version(MajorVersionContext, MinorVersionContext) 
		>= version(MajorVersionRequire, MinorVersionRequire);
}

void gameFramework::cursorPositionCallback(GLFWwindow* Window, double x, double y)
{
	gameFramework * Test = static_cast<gameFramework*>(glfwGetWindowUserPointer(Window));
	assert(Test);

	Test->MouseCurrent = glm::ivec2(x, y);
	Test->TranlationCurrent = Test->MouseButtonFlags & gameFramework::MOUSE_BUTTON_LEFT ? Test->TranlationOrigin + (Test->MouseCurrent - Test->MouseOrigin) / 10.f : Test->TranlationOrigin;
	Test->RotationCurrent = Test->MouseButtonFlags & gameFramework::MOUSE_BUTTON_RIGHT ? Test->RotationOrigin + glm::radians(Test->MouseCurrent - Test->MouseOrigin) : Test->RotationOrigin;
}

void gameFramework::mouseButtonCallback(GLFWwindow* Window, int Button, int Action, int mods)
{
	gameFramework * Test = static_cast<gameFramework*>(glfwGetWindowUserPointer(Window));
	assert(Test);

	switch(Action)
	{
		case GLFW_PRESS:
		{
			Test->MouseOrigin = Test->MouseCurrent;
			switch(Button)
			{
				case GLFW_MOUSE_BUTTON_LEFT:
				{
					Test->MouseButtonFlags |= gameFramework::MOUSE_BUTTON_LEFT;
					Test->TranlationOrigin = Test->TranlationCurrent;
				}
				break;
				case GLFW_MOUSE_BUTTON_MIDDLE:
				{
					Test->MouseButtonFlags |= gameFramework::MOUSE_BUTTON_MIDDLE;
				}
				break;
				case GLFW_MOUSE_BUTTON_RIGHT:
				{
					Test->MouseButtonFlags |= gameFramework::MOUSE_BUTTON_RIGHT;
					Test->RotationOrigin = Test->RotationCurrent;
				}
				break;
			}
		}
		break;
		case GLFW_RELEASE:
		{
			switch(Button)
			{
				case GLFW_MOUSE_BUTTON_LEFT:
				{
					Test->TranlationOrigin += (Test->MouseCurrent - Test->MouseOrigin) / 10.f;
					Test->MouseButtonFlags &= ~gameFramework::MOUSE_BUTTON_LEFT;
				}
				break;
				case GLFW_MOUSE_BUTTON_MIDDLE:
				{
					Test->MouseButtonFlags &= ~gameFramework::MOUSE_BUTTON_MIDDLE;
				}
				break;
				case GLFW_MOUSE_BUTTON_RIGHT:
				{
					Test->RotationOrigin += glm::radians(Test->MouseCurrent - Test->MouseOrigin);
					Test->MouseButtonFlags &= ~gameFramework::MOUSE_BUTTON_RIGHT;
				}
				break;
			}
		}
		break;
	}
}

void gameFramework::keyCallback(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods)
{
	if (Key < 0)
		return;

	gameFramework * Test = static_cast<gameFramework*>(glfwGetWindowUserPointer(Window));
	assert(Test);

    // ignore "repeat" events when key is already down
    if (Action == KEY_REPEAT) {
        return;
    }

	Test->KeyPressed[Key] = Action == KEY_PRESS;

	if(Test->isKeyPressed(GLFW_KEY_ESCAPE))
		Test->stop();
}

void APIENTRY gameFramework::debugOutput
(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam
)
{
	assert(userParam);
	gameFramework* Test = static_cast<gameFramework*>(const_cast<GLvoid*>(userParam));
	
	char debSource[32], debType[32], debSev[32];

	if(source == GL_DEBUG_SOURCE_API_ARB)
		strcpy(debSource, "OpenGL");
	else if(source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
		strcpy(debSource, "Windows");
	else if(source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
		strcpy(debSource, "Shader Compiler");
	else if(source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
		strcpy(debSource, "Third Party");
	else if(source == GL_DEBUG_SOURCE_APPLICATION_ARB)
		strcpy(debSource, "Application");
	else if (source == GL_DEBUG_SOURCE_OTHER_ARB)
		strcpy(debSource, "Other");
	else
		assert(0);
 
	if(type == GL_DEBUG_TYPE_ERROR)
		strcpy(debType, "error");
	else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
		strcpy(debType, "deprecated behavior");
	else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		strcpy(debType, "undefined behavior");
	else if(type == GL_DEBUG_TYPE_PORTABILITY)
		strcpy(debType, "portability");
	else if(type == GL_DEBUG_TYPE_PERFORMANCE)
		strcpy(debType, "performance");
	else if(type == GL_DEBUG_TYPE_OTHER)
		strcpy(debType, "message");
	else if(type == GL_DEBUG_TYPE_MARKER)
		strcpy(debType, "marker");
	else if(type == GL_DEBUG_TYPE_PUSH_GROUP)
		strcpy(debType, "push group");
	else if(type == GL_DEBUG_TYPE_POP_GROUP)
		strcpy(debType, "pop group");
	else
		assert(0);
 
	if(severity == GL_DEBUG_SEVERITY_HIGH_ARB)
	{
		strcpy(debSev, "high");
		if(Test->Success == GENERATE_ERROR || source != GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
			Test->Error = true;
	}
	else if(severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
		strcpy(debSev, "medium");
	else if(severity == GL_DEBUG_SEVERITY_LOW_ARB)
		strcpy(debSev, "low");
	else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		strcpy(debSev, "notification");
	else
		assert(0);

	fprintf(stderr,"%s: %s(%s) %d: %s\n", debSource, debType, debSev, id, message);

	if(Test->Success != GENERATE_ERROR && source != GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
		assert(!Test->Error);
}

