void KeyboardHandle(struct TrapFrame *tf)
{
    uint32_t code = getKeyCode();
    if (code == 0xe)
    { // 退格符
        // TODO: 要求只能退格用户键盘输入的字符串，且最多退到当行行首
        if (displayCol && deleteBack(&inputBuf))
        {
            --displayCol;
            updateCursor(displayRow, displayCol);
            printChar(' ', displayCol, displayCol);
        }
    }
    else if (code == 0x1c)
    { // 回车符
        // TODO: 处理回车情况
        if (displayRow == 24)
            scrollScreen();
        else
            ++displayRow;
        displayCol = 0;
        insertBuf(&inputBuf, '\n');
        putChar('\n');
    }
    else if (code < 0x81 && (code > 1 && code < 0xe || code > 0xf && code != 0x1d && code != 0x2a && code != 0x36 && code != 0x38 && code != 0x3a && code < 0x45))
    { // 正常字符
        // TODO: 注意输入的大小写的实现、不可打印字符的处理
        putChar(getChar(code));
        printChar(getChar(code), displayRow, displayCol);
        insertBuf(&inputBuf, getChar(code));
        if (displayCol == 79)
        {
            displayCol = 0;
            if (displayRow == 24)
                scrollScreen();
            else
                ++displayRow;
        }
        else
            ++displayCol;
    }
    updateCursor(displayRow, displayCol);
}