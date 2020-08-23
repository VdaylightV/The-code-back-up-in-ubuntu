#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

// #define DEBUG
#define Max_Output_Num 8

extern char **environ;

typedef struct Func_Time
{
  char *func;
  double time;
  struct Func_Time* next;
} func_time;


void show(func_time *head, double total_time);
void Free(func_time *head);
void sort(func_time *head);
func_time *append(func_time *head, func_time *p);
void clear(char *s, int count);

int main(int argc, char *argv[]) {
  #ifdef DEBUG
  printf("argv:\n");
  for (int i = 0; i < argc; i++){
    printf("%d: %s\n",i, argv[i]);
  }
  printf("\n");
  #endif

  int fildes[2];
  if(pipe(fildes) != 0){
    fprintf(stderr, "pipe failed\n");
    exit(1);
  };

  #ifdef DEBUG
  printf("pipe info:\nfildes[0] = %d, fildes[1] = %d\n\n", fildes[0], fildes[1]);
  #endif

  int rc = fork();
  if (rc < 0){
    // fork failed
    fprintf(stderr, "fork failed\n");
    exit(1);
  }
  else if (rc == 0)
  {
    // Child process

    // 获取参数
    char *exec_argv[argc+2];
    // 先将所有的argv中的参数拷贝过来
    for (size_t i = 0; i < argc; i++)
    {
      exec_argv[i+1] = argv[i];
    }
    
    exec_argv[0] = "strace"; // 原来的argv[0]为 "sperf-32/64",现直接将它改为strace
    exec_argv[1] = "-T"; // 添加 "-T" 选项
    exec_argv[argc + 1] = NULL; // exec_argv = {"strace", "-T", "ls", ..., NULL}

    #ifdef DEBUG
    printf("exec_argv:\n");
    for (size_t i = 0; exec_argv[i] != NULL; i++)
    {
      printf("%s\n", exec_argv[i]);
    }
    printf("\n");
    #endif

    // 文件描述符的腾挪
    close(fildes[0]);
    dup2(fildes[1], STDERR_FILENO);
    int fd = open("/dev/null", O_WRONLY);
    dup2(fd, STDOUT_FILENO);

    // 检索PATH
    char *all_path_temp = getenv("PATH");
    char all_path[strlen(all_path_temp) + 1];
    strcpy(all_path, all_path_temp);

    #ifdef DEBUG
    printf("all_path = %s\n", all_path);
    #endif

    // // 用于 exec_envp
    // char PATH[500] = "PATH=";
    // strcat(PATH, all_path);

    // // 设置执行的环境变量
    // char *exec_envp[] = { PATH, NULL, };

    // 获取 PATH 环境变量中的每一个PATH
    char *path = strtok(all_path, ":");
    while (path != NULL)
    {
      char filename[strlen(path)+8];
      strcpy(filename, path);
      strcat(filename, "/strace");
      // filename = ${PATH}/strace

      #ifdef DEBUG
      // printf("PATH = %s\n", path);
      printf("filename = %s\n", filename);

      // for (size_t i = 0; environ[i] != NULL; i++)
      // {
      //   printf("%s\n", environ[i]);
      // }
      #endif

      execve(filename, exec_argv, environ);
      path = strtok(NULL, ":");
    }
  }
  else
  {
    // Parent goes down this path
    close(fildes[1]);
    char s[300] = {'\0'};
    dup2(fildes[0], STDIN_FILENO);

    func_time *head = NULL;
    double total_time = 0;

    clock_t begin = clock();

    while (fgets(s, 300, stdin) != NULL)
    {
      #ifdef DEBUG
      printf("%s", s);
      #endif

      // Get function name
      size_t index = 0;

      // 首先检查第一个字符是否为字母,去除类似于"+++ exit +++
      if (!((s[index] >= 'a') && (s[index] <= 'z')))
      {
        continue;
      }
      
      for (; s[index] != '('; index++);
      char *func = malloc(index + 1);
      memcpy(func, s, index);
      func[index] = '\0';
      if (strcmp(func, "exit_group") == 0)
      {
        continue;
      }
      

      index = 300-1;
      for (; index != 0; index--)
      {
        if (s[index] == '<')
        {
          break;
        }
      }
      while (index == 0)
      {
        clear(s, 300);
        fgets(s, 300, stdin);
        index = 300-1;
        for (; index != 0; index--)
        {
          if (s[index] == '<')
          {
            break;
          }
        }
      }

      // Get syscall time
      char time_str[9];
      memcpy(time_str, &s[index + 1], 8);
      time_str[8] = '\0';
      double time;
      sscanf(time_str, "%lf", &time);
      total_time += time;

      clear(s, 300);

      #ifdef DEBUG
      printf("Get function name and time.\n");
      printf("func: %s, time: %.6lf\n", func, time);
      printf("\n");
      #endif

      func_time *p = malloc(sizeof(func_time));
      p->func = func;
      p->time = time;

      head = append(head, p);
      sort(head);

      clock_t current = clock();
      if (current - begin > CLOCKS_PER_SEC/10)
      {
        begin = current;
        show(head, total_time);
      }
    }

    sort(head);
    #ifdef DEBUG
    func_time *q = head;
    while (q != NULL)
    {
      printf("func: %s, time: %.6lf\n", q->func, q->time);
      q = q->next;
    }
    #endif
    show(head, total_time);
    Free(head);
  }

  // execve("strace",          exec_argv, exec_envp);
  // execve("/bin/strace",     exec_argv, exec_envp);
  // execve("/usr/bin/strace", exec_argv, exec_envp); 
  // perror(argv[0]);
  // exit(EXIT_FAILURE);
  return 0; 
}


// Actually append or add
func_time *append(func_time *head, func_time *p){
  if (head == NULL)
  {
    head = p;
    head->next = NULL;
    return head;
  }

  func_time *q = head;
  while (q->next != NULL)
  {
    if (strcmp(q->func, p->func) == 0)
    {
      q->time += p->time;
      return head;
    }
    q = q->next;
  }
  
  if (strcmp(q->func, p->func) == 0)
  {
    q->time += p->time;
    return head;
  }
    
  q->next = p;
  p->next = NULL;
  return head;
}

void sort(func_time *head){
  assert(head != NULL);
  func_time *p = head;

  for (; p->next != NULL; p = p->next)
  {
    func_time *q = p->next;
    while (q != NULL)
    {
      if (q->time > p->time)
      {
        func_time temp = *q;
        q->func = p->func;
        q->time = p->time;
        p->func = temp.func;
        p->time = temp.time;
      }
      q = q->next;
    }
  }
}

void Free(func_time *head){
  func_time *p= head;
  while (p != NULL)
  {
    head = p->next;
    free(p->func);
    free(p);
    p = head;
  }
}

void show(func_time *head, double total_time){
  assert(head != NULL);
  // static unsigned int counter = 1;
  // printf("Time #%d\n", counter);
  int num = 1;
  for(func_time *p = head; p != NULL; p = p->next){
    printf("%s (%d%%)\n", p->func, (unsigned int)(p->time/total_time*100));
    if (num == Max_Output_Num)
    {
      break;
    }
    num++;
  }
  printf("==============================\n");
  for (size_t i = 0; i < 80; i++)
  {
    printf("%c", 0);
  }
  
  fflush(stdout);
}

void clear(char *s, int count){
  for (size_t i = 0; i < count; i++)
  {
    s[i] = '\0';
  }
}