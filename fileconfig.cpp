
#include "util.h"
#include "logger.h"

// 복사 연산자
CKeyset& CKeyset::operator=(const CKeyset& t)
{
	reset();
#if USE_KEYSET_ARRAY
	int i;
	for (i = 0; i < t.key_count; i++) {
		this->add(t.keyvalue[i].key, t.keyvalue[i].value);
	}
	this->key_count = t.key_count;
#else
	list<key_value*>::const_iterator it_kv;
	for (it_kv = t.keyvalue.begin(); it_kv != t.keyvalue.end(); it_kv++) {
		key_value* pkv = *it_kv;
		add(pkv->key, pkv->value);
	}
#endif
	CKeyset* src = (CKeyset*)&t;	// 바로 억세스하면 컴파일상 워닝 메시지 발생...
	const char* ks = src->getks();	// 바로 억세스하면 컴파일상 워닝 메시지 발생...

	fileconfig_key = *ks ? strdup(ks) : NULL;

	return *this;
}

int CKeyset::add(const char* key, const char* value)
{
	if (!key || !*key)	// 키가 없음
		return -1;
	if (*getvalue(key))		// 키 중복
		return -2;

#if USE_KEYSET_ARRAY
	if (key_count >= MAX_KEY_COUNT)
		return -1;
	keyvalue[key_count].key = strdup(key);
	keyvalue[key_count].value = *value ? strdup(value) : NULL;
	return ++key_count;
#else
	key_value* pkv = new key_value;
	pkv->key = strdup(key);
	pkv->value = *value ? strdup(value) : NULL;
	keyvalue.push_back(pkv);
	return keyvalue.size();
#endif
}

const char* CKeyset::getvalue(const char* key)
{
#if USE_KEYSET_ARRAY
	int i;
	for (i = 0; i < key_count; i++) {
		if (!strcmp(keyvalue[i].key, key))
			return keyvalue[i].value;
	}
#else
	list<key_value*>::iterator it_kv;
	for (it_kv = keyvalue.begin(); it_kv != keyvalue.end(); it_kv++) {
		key_value* pkv = *it_kv;
		if (!strcmp(pkv->key, key))
			return pkv->value;
	}
#endif
	return "";
}

const char* CKeyset::getvalue(size_t nIndx)
{
#if USE_KEYSET_ARRAY
	if (nIndx >= key_count)
		return "";

	return keyvalue[nIndx].value;
#else
	if (nIndx >= keyvalue.size())
		return "";

	list<key_value*>::iterator it_kv;
	size_t i;
	for (it_kv = keyvalue.begin(), i = 0; it_kv != keyvalue.end(); it_kv++, i++) {
		if (i == nIndx) {
			key_value* pkv = *it_kv;
			return pkv->value;
		}
	}
#endif
	return "";
}

const char* CKeyset::getkey(size_t nIndx)
{
#if USE_KEYSET_ARRAY
	if (nIndx >= key_count)
		return NULL;

	return keyvalue[nIndx].key;
	
#else
	if (nIndx >= keyvalue.size())
		return NULL;

	list<key_value*>::iterator it_kv;
	size_t i;
	for (it_kv = keyvalue.begin(), i = 0; it_kv != keyvalue.end(); it_kv++, i++) {
		if (i == nIndx) {
			key_value* pkv = *it_kv;
			return pkv->key;
		}
	}
#endif
	return NULL;
}

void CKeyset::reset()
{
	if (fileconfig_key) {
		free(fileconfig_key);
		fileconfig_key = NULL;
	}
#if USE_KEYSET_ARRAY
	if (!key_count) return;

	int i;
	for (i = 0; i < key_count; i++) {
		if (keyvalue[i].key) free(keyvalue[i].key);
		if (keyvalue[i].value) free(keyvalue[i].value);
	}
	bzero(this, sizeof(*this));
#else
	list<key_value*>::iterator it_kv;
	for (it_kv = keyvalue.begin(); it_kv != keyvalue.end(); it_kv++) {
		key_value* pkv = *it_kv;
		if (pkv->key) free(pkv->key);
		if (pkv->value) free(pkv->value);
		free(pkv);
	}
	keyvalue.clear();
#endif
}

CFileConfig::CFileConfig() 
{
	m_config.clear();
}

CFileConfig::~CFileConfig()
{
	map<const char*, CKeyset*>::iterator it_ks;
	for (it_ks = m_config.begin(); it_ks != m_config.end(); it_ks++) {
		delete it_ks->second;
	}
	m_config.clear();
}

// 복사 연산자
CFileConfig& CFileConfig::operator=(const CFileConfig& t)
{
	m_config.clear();
	map<const char*, CKeyset*>::const_iterator it_ks;
	for (it_ks = t.m_config.begin(); it_ks != t.m_config.end(); it_ks++) {
		this->m_config[it_ks->first] = new CKeyset(*it_ks->second);
	}
	return *this;
}

int CFileConfig::load(const char* file)
{
	const char* delimiter = "\t \r\n";
	char buf[1024] = { 0, };
	ifstream fs;
	size_t len;
	char* ptr, * key, * value;
	char szSection[128] = { 0, };
	CKeyset ks;
	map<const char*, CKeyset> it_config;

	/* --- 구성 -----
	* [section1]
	* key1= value1
	* key2: value2
	* [section2]
	* key1= value1
	* key2: value2
	* '#' 이후 문장은 comment
	* 위 구조의 파일을 section 별 key,value 페어로 파일에서 로딩한다
	*/

	if (*file == '~')
		snprintf(buf, sizeof(buf) - 1, "%s%s", getenv("HOME"), file + 1);
	else
		snprintf(buf, sizeof(buf) - 1, "%s", file);

	fs.open(buf, ios_base::in);

	if (fs.fail()) {
		conpt("file open failed %s ", file);
		return -1;
	}

	conpt("-----\nopen fileconfig %s ", buf);

	try
	{
		int nLine = 0;
		while (true)
		{
			*buf = 0x00;
			fs.getline(buf, sizeof(buf) - 1);
			if (fs.eof())
				break;
			len = strlen(buf);

			conpt("file read line %d, len=%d, data=%s", ++nLine, len, buf);

			ptr = strchr(buf, '#');
			if (ptr)
				*ptr = '\0';

			if (!*buf) continue;
			strltrim(buf, delimiter); // white space ltrim
			if (!*buf) continue;
			strrtrim(buf, delimiter); // white space rtrim

			// 새로운 queue 정보 시작인지 확인
			// 새로운 queue가 시작 되었다면 지금까지 수집된 queue struct 를 저장한다
			len = strlen(buf);
			if (*buf != '[' || buf[len-1] != ']') {
				// 첫번째 섹션이 나오기 전까지는 모두 주석처리한다.
				if (!*szSection)
					continue;

				// 현재 섹션에 keyvalue 를 추가한다
				// key 및 value 가 쌍으로 한라인에 있어야 한다.
				key = buf;
				value = strpbrk(buf, "=:");  // {'=',':'} 2개 중 아무거난 구분자로 사용할 수 있다.
				// 구분자가 없으면 이 라인정보는 무시한다
				if (!value)
					continue;

				*value++ = '\0';
				strrtrim(key, delimiter);
				strltrim(value, delimiter); // 널값이 설정될 수 있다.

				if (!*key) {
					throw util_exception(130, "key 값은 널이 올 수 없습니다.");
				}

				ks.add(key, value);

				continue;
			}

			// 섹션 이 바뀌었다... 

			buf[len - 1] = '\0';
			ptr = buf + 1;
			strltrim(ptr, delimiter); // 널값이 설정될 수 있다.
			if (!*ptr) {
				// 섹션명이 아제 지정되지 않았다. 뭐꼬 이런 오류까지 검사하게 하냐?
				throw util_exception(140, "section 값은 널이 올 수 없습니다.");
			}
			strcpy(szSection, ptr);

			if ( !strcmp(szSection, ks.getks()) || m_config.find(szSection) != m_config.end()) {
				throw util_exception(120, "%s section 은 중복되었습니다.", szSection);
			}

			if (ks.getcount()) {
				// 앞전 데이타가 있다 처리하자
				CKeyset* pks = new CKeyset(ks);
				m_config[pks->getks()] = pks;
#ifdef DEBUG
				conpt("m_config size is %d\n", m_config.size());
#endif
			}
			ks.reset();
			ks.setks(szSection);
		}

		if (ks.getcount()) {
			// 마지막 데이타가 있다 처리하자, key 중복체크는 이미 했다.
			CKeyset* pks = new CKeyset(ks);
			m_config[pks->getks()] = pks;
#ifdef DEBUG
			conpt("m_config size is %d\n", m_config.size());
#endif
		}
		fs.close();
		conpt("-----\nclose fileconfig %s", basename(file));
	}
	catch (util_exception& e) {
		conpt("errno=%d, %s", e.code(), e.what());
		return -1;
	}
	catch (...) {
		conpt("errno=%d, %s", errno, strerror(errno));
		return -1;
	}

	return m_config.size();

}




