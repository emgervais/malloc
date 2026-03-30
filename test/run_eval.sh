#!/bin/bash

# ==============================================================================
#                  42 Malloc Project Evaluation Script (v2 - Patched)
# ==============================================================================
# 이 스크립트는 평가 시트의 모든 테스트를 자동으로 수행합니다.
# 프로젝트 루트 디렉토리에서 실행하세요. (./run_evaluation.sh)
# ==============================================================================

# --- 출력용 색상 설정 ---
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
RED="\033[0;31m"
BLUE="\033[0;34m"
RESET="\033[0m"

# --- 운영체제 감지 및 설정 ---
OS_NAME=$(uname -s)
TIME_CMD="/usr/bin/time"
TIME_FLAG="-l" # macOS 기본값

if [ "$OS_NAME" = "Linux" ]; then
    TIME_FLAG="-v"
    if ! command -v $TIME_CMD &> /dev/null; then
        echo -e "${RED}오류: /usr/bin/time 이 설치되지 않았습니다. 'sudo apt install time' 명령어로 설치해주세요.${RESET}"
        exit 1
    fi
fi

# --- 헬퍼 함수 ---
print_header() {
    echo -e "\n${BLUE}=====================================================${RESET}"
    echo -e "${BLUE}  $1${RESET}"
    echo -e "${BLUE}=====================================================${RESET}"
}

run_cmd() {
    echo -e "${YELLOW}> $@${RESET}"
    eval "$@"
}

check_success() {
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ 성공${RESET}"
    else
        echo -e "${RED}✗ 실패${RESET}"
    fi
}

# --- 테스트 파일 자동 생성 함수 ---
create_test_files() {
    print_header "1. 평가용 C 테스트 파일 자동 생성"

    # test0.c: 기본 프로그램
    cat << 'EOF' > test0.c
int main() { return 0; }
EOF

    # test1.c: 1MB 할당
    cat << 'EOF' > test1.c
#include <stdlib.h>
#include <string.h>
int main() { int i; char *addr; for (i = 0; i < 1024; i++) { addr = (char*)malloc(1024); addr[0] = 42; } return 0; }
EOF

    # test2.c: 할당 후 모두 free
    cat << 'EOF' > test2.c
#include <stdlib.h>
#include <string.h>
int main() { int i; char *addr; for (i = 0; i < 1024; i++) { addr = (char*)malloc(1024); addr[0] = 42; free(addr); } return 0; }
EOF
    # test3.c: realloc 기본
    cat << 'EOF' > test3.c
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main() { char *addr1; char *addr2; addr1 = (char*)malloc(16 * 1024 * 1024); strcpy(addr1, "Bonjour"); write(1, addr1, 8); addr2 = (char*)realloc(addr1, 128 * 1024 * 1024); addr2[127 * 1024 * 1024] = 42; write(1, addr2, 8); write(1, "\n", 1); return 0; }
EOF

    # test4.c: realloc 확장/축소
    cat << 'EOF' > test4.c
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main() { char *addr; addr = malloc(16); strcpy(addr, "START"); addr = realloc(addr, 128); if (strcmp(addr, "START") == 0) { write(1, "OK\n", 3); } else { write(1, "KO\n", 3); } addr = realloc(addr, 4); if (strncmp(addr, "STAR", 4) == 0) { write(1, "OK\n", 3); } else { write(1, "KO\n", 3); } free(addr); return 0; }
EOF

    # test5.c: realloc 오류 처리 (수정된 버전)
    cat << 'EOF' > test5.c
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main() { char *addr; char *addr2; addr = malloc(10); strcpy(addr, "Bonjour"); realloc(addr, 0); /* This should act like free(addr) */ addr2 = realloc(NULL, 20); strcpy(addr2, "World"); write(1, addr2, 6); write(1, "\n", 1); free(addr2); return 0; }
EOF

    # test6.c: show_alloc_mem
    cat << 'EOF' > test6.c
#include <stdlib.h>
void show_alloc_mem();
int main() { char *addr1; char *addr2; char *addr3; addr1 = (char*)malloc(10); addr2 = (char*)malloc(500); addr3 = (char*)malloc(5000); show_alloc_mem(); return 0; }
EOF
    echo -e "${GREEN}✓ 모든 테스트 파일 생성 완료.${RESET}"
}

# --- 라이브러리 로더 스크립트 자동 생성 함수 ---
create_run_script() {
    print_header "2. 라이브러리 로더 (run.sh) 자동 생성"
    if [ "$OS_NAME" = "Linux" ]; then
        echo '#!/bin/sh
export LD_LIBRARY_PATH=.
export LD_PRELOAD=./libft_malloc.so
$@' > run.sh
    else # macOS
        echo '#!/bin/sh
export DYLD_LIBRARY_PATH=.
export DYLD_INSERT_LIBRARIES="libft_malloc.so"
export DYLD_FORCE_FLAT_NAMESPACE=1
$@' > run.sh
    fi
    chmod +x run.sh
    echo -e "${GREEN}✓ ${OS_NAME} 환경에 맞는 run.sh 생성 완료.${RESET}"
}

# ==============================================================================
#                              테스트 시작
# ==============================================================================

# --- 파일 생성 ---
create_test_files
create_run_script

# --- 사전 준비: 컴파일 ---
print_header "3. 사전 준비 (Preliminary Checks)"
run_cmd "make fclean && make"
check_success

# --- 라이브러리 컴파일 테스트 ---
print_header "4. 라이브러리 컴파일 테스트"
run_cmd "export HOSTTYPE=Testing && make re"
check_success
echo -e "${YELLOW}> ls -l libft_malloc.so${RESET}"
ls -l libft_malloc.so
echo -e "${YELLOW}[수동 확인] 위에 'libft_malloc.so -> libft_malloc_Testing.so'가 출력되었는지 확인하세요.${RESET}"

# --- 함수 Export 테스트 ---
print_header "5. 함수 Export 테스트"
run_cmd "nm libft_malloc.so | grep -E ' T _(malloc|free|realloc|show_alloc_mem)'"
echo -e "${YELLOW}[수동 확인] 위에 malloc, free, realloc, show_alloc_mem 함수가 'T'와 함께 출력되었는지 확인하세요.${RESET}"

# --- 테스트 바이너리 컴파일 ---
print_header "6. 테스트 바이너리 컴파일"
run_cmd "gcc -o test0 test0.c" && check_success
run_cmd "gcc -o test1 test1.c" && check_success
run_cmd "gcc -o test2 test2.c" && check_success
run_cmd "gcc -o test3 test3.c" && check_success
run_cmd "gcc -o test4 test4.c" && check_success
run_cmd "gcc -o test5 test5.c" && check_success
# 수정된 컴파일 명령어
run_cmd "gcc -o test6 test6.c -L. -lft_malloc" && check_success

# --- 기능 테스트 ---
print_header "7. 기능 테스트"

# echo -e "\n${YELLOW}--- 7.1 Malloc 테스트 (시스템 Malloc) ---${RESET}"
# echo -e "${BLUE}실행: test0 (기준값)...${RESET}"
# run_cmd "$TIME_CMD $TIME_FLAG ./test0"
# echo -e "\n${BLUE}실행: test1 (1MB 할당)...${RESET}"
# run_cmd "$TIME_CMD $TIME_FLAG ./test1"
# echo -e "${YELLOW}[수동 확인] test0과 test1의 'maximum resident set size' 또는 'Minor (reclaiming a frame) page faults' 값을 비교하세요.${RESET}"

echo -e "\n${YELLOW}--- 7.2 Malloc 테스트 (구현한 Malloc) ---${RESET}"
echo -e "${BLUE}실행: test0 (라이브러리 적용 기준값)...${RESET}"
run_cmd "./run.sh $TIME_CMD $TIME_FLAG ./test0"
echo -e "\n${BLUE}실행: test1 (라이브러리 적용 1MB 할당)...${RESET}"
run_cmd "./run.sh $TIME_CMD $TIME_FLAG ./test1"
echo -e "${YELLOW}[수동 확인] 위 두 결과의 페이지 차이를 계산하여 평가지의 점수 기준과 비교하세요.${RESET}"

echo -e "\n${YELLOW}--- 7.3 Free 테스트 ---${RESET}"
run_cmd "./run.sh $TIME_CMD $TIME_FLAG ./test2"
echo -e "${YELLOW}[수동 확인] 이 테스트의 'page reclaims' 값이 7.2의 test1보다 적어야 합니다.${RESET}"
echo -e "${YELLOW}[수동 확인] 이 테스트의 'page reclaims' 값이 7.2의 test0보다 최대 3 많아야 합니다.${RESET}"

echo -e "\n${YELLOW}--- 7.4 Realloc 테스트 ---${RESET}"
run_cmd "./run.sh ./test3"
check_success

echo -e "\n${YELLOW}--- 7.5 Realloc++ 테스트 ---${RESET}"
run_cmd "./run.sh ./test4"
check_success

echo -e "\n${YELLOW}--- 7.6 오류 관리 테스트 ---${RESET}"
run_cmd "./run.sh ./test5"
check_success

echo -e "\n${YELLOW}--- 7.7 show_alloc_mem 테스트 ---${RESET}"
run_cmd "./run.sh ./test6"
echo -e "${YELLOW}[수동 확인] 위의 메모리 맵 출력이 과제 설명서의 형식과 일치하는지 확인하세요.${RESET}"


print_header "평가 스크립트 종료"
echo "일부 항목은 위에 출력된 결과를 직접 비교하여 확인해야 합니다."