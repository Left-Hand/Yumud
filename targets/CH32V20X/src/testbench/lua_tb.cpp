#include "tb.h"

extern "C"{
#include "../thirdparty/lua/lauxlib.h"  
#include "../thirdparty/lua/lualib.h"  
#include "../thirdparty/lua/lua.h"
}

const char lua_test[] = {
    "print(\"Hello,I am lua!\\n--this is newline printf\")\n"
    "function foo()\n"
    "  local i = 0\n"
    "  local sum = 1\n"
    "  while i <= 10 do\n"
    "    sum = sum * 2\n"
    "    i = i + 1\n"
    "  end\n"
    "return sum\n"
    "end\n"
    "print(\"sum =\", foo())\n"
    "print(\"and sum = 2^11 =\", 2 ^ 11)\n"
};

void lua_tb(OutputStream & logger){
    // lua_State * L = luaL_newstate();
    // luaL_openlibs(L);
    // lua_tb_open(L);
    // lua_pcall(L, 0, 0, 0);
    // lua_close(L);

	// lua_State *L;
	// L = lua_newstate(); 			//	����Lua������
	// luaopen_base(L);				//	ע���������
	// lua_dostring(L, lua_test); 	//	ִ�нű����
	// return 0;


	//1.����һ��state  
	// luaL_newstate����һ��ָ���ջ��ָ��
	lua_State *L = luaL_newstate();
 
	//2.��ջ����  
	lua_pushstring(L, "I am so cool~");
	lua_pushnumber(L, 20);
 
	//3.ȡֵ����  
	if (lua_isstring(L, 1)) {             //�ж��Ƿ����תΪstring  
		logger.println(lua_tostring(L, 1));  //תΪstring������  
	}
	if (lua_isnumber(L, 2)) {
		logger.println(lua_tostring(L, 2));
	}

	//4.�ر�state  
	lua_close(L);
}