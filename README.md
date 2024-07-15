# stm32h750vb
基于 zephyr 学习STM32工程

west build
west flash  // 使用 st-link 下载
west flash --runner jlink 

### create a new repository on the command line
- git pull (如果是首次推送，先下拉然后把.git复制到工程里面)
- yes | rm -r .git
- git init
- git add .
- git add README.md
- git commit -m "first commit"
- git branch -M master
- git remote remove origin   (删除远程链接)
- git push -u origin master