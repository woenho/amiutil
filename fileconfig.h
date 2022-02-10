
/* string으로 처리하면 프린터출력, sprintf, 통신버퍼에 복사등을 할 때
* char*로 변환해 주어야하는 상황이 지속적으로 발생한다
* 그래서 그냥 char* 로 처리하는걸 맹근다
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <dirent.h>
#include <syslog.h>

// c++
#include <map>
#include <list>
#include <queue>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#define USE_KEYSET_ARRAY		(0)

typedef struct key_value_t {
	char* key;
	char* value;
}key_value;

class CKeyset {
#if USE_KEYSET_ARRAY
	int key_count;
public:
#define MAX_KEY_COUNT	(50)
	key_value keyvalue[MAX_KEY_COUNT];
	CKeyset() { bzero(this, sizeof(*this)); }
	CKeyset(const CKeyset& t) { bzero(this, sizeof(*this)); *this = t; }	// 복사 생성자
#else
public:
	string config_key;
	list<key_value*> keyvalue;	// * 를 사용한것은 key_value의 각종 연산자함수를 추가하지 않고 사용하기 위해서이다.
	CKeyset() { keyvalue.clear(); }
	CKeyset(const CKeyset& t){ keyvalue.clear(); *this = t; }	// 복사 생성자
#endif
	~CKeyset() { reset(); }
	
	// operator
	CKeyset& operator=(const CKeyset& t);	// 복사 대입 연산자(copy assignment operator)
	int add(const char* key, const char* value);
	void reset();
	const char* getvalue(const char* key);
	const char* getvalue(size_t nIndx);
	const char* getkeyname(size_t nIndx);
	string getks() { return config_key; }
	void setks(const char* key) { config_key = key; }

	// access to member
#if USE_KEYSET_ARRAY
	int getcount() { return key_count; }
#else
	int getcount() { return keyvalue.size(); }	
#endif

};

class CFileConfig {
public:
	map<string, CKeyset*> m_config; // CKeyset 클래스를 사용하지 않고 CKeyset*를 이용하는 이유는 자주발생되는 복사생성자의 실행을 줄이기 위해서다
	CFileConfig();
	~CFileConfig();
	CFileConfig(const CFileConfig& t) { bzero(this, sizeof(*this));	m_config = t.m_config; }	// 복사 생성자

	// operator
	CFileConfig& operator=(const CFileConfig& t);	// 복사 대입 연산자(copy assignment operator)
	int load(const char* file);
};


#endif
