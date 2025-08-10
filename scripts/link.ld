/* 以 gcc 默认脚本为基础再插入我们自己的段 */
SECTIONS
{
    /* 把 .my_table 放在 .data 之前，保证连续 */
    .my_table ALIGN(8) : {
        __start_my_table = .;   /* 起始符号 */
        KEEP(*(SORT_BY_NAME(.my_table)))      /* 把所有输入文件的 .my_table 段合进来 */
        __stop_my_table  = .;   /* 结束符号 */
    }

    /* 其余段保持默认，用 INSERT AFTER 或 INCLUDE 默认脚本都行 */
}
INSERT AFTER .data;
