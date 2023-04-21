cmd_/home/user/shared/lunix-tng/modules.order := {   echo /home/user/shared/lunix-tng/lunix.ko; :; } | awk '!x[$$0]++' - > /home/user/shared/lunix-tng/modules.order
