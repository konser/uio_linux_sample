#/bin/sh
echo $1
if [ "$1" = "0" ]
then
    export LANG="en_US.UTF-8"
    unset http_proxy
    unset https_proxy
    export http_proxy="http://10.48.19.179:8866"
    export https_proxy="https://10.48.19.179:8866"
    git config --global http.proxy http://10.48.:8866
    git config --global https.proxy https://10.48:8866
    export GIT_COMMITTER_EMAIL=wu860403@gmail.com
    export GIT_AUTHOR_EMAIL=wu860403@gmail.com
    export LANG="en_US.UTF-8"
    #export LANG="zh_CN.UTF-8"
elif [ "$1" = "1" ]
then
    unset http_proxy
    unset https_proxy
    #curl --noproxy '*' -v -d "opr=pwdLogin&userName=18086376&pwd=Sn@12345&&rememberPwd=0" http://10.37.235.33/ac_portal/login.php
    export http_proxy="http://10.37.:8080"
    export https_proxy="http://10.37.:8080"
elif [ "$1" = "2" ]
then
    unset http_proxy
    unset https_proxy
else
    unset http_proxy
    unset https_proxy
    echo google
    export http_proxy="http://10.20:888"
    export https_proxy="http://10.20:888"
    git config --global http.proxy http://10.:888
    git config --global https.proxy https://10:888
fi
