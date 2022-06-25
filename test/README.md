# Test说明
------
### 说明
1 从一个任务Give，满负荷向另一个任务，同时从0.8ms中断中和高优先级任务中，发送纯事件。
2-0 从一个任务Give，满负荷向另一个任务Value，同时从0.8ms中断中和高优先级任务Middle和High中，发布订阅的纯事件。
2-1 从一个任务Give，满负荷向状态机Sm，同时从0.8ms中断中和高优先级任务High与Middle中，发布订阅的纯事件。
2-2 从一个任务Give，满负荷向状态机Sm和另一个任务Value，同时从0.8ms中断中，和高优先级任务High与Middle中，发布订阅的纯事件。
3 从一个任务Give，满负荷向另一个任务Value，同时从0.8ms中断中，和高优先级任务High与Middle中，发送流事件。
4 从一个任务Give，满负荷向数据库Value，同时从0.8ms中断中，和高优先级任务High与Middle中，发送数据，从另一个任务读取。
5 从一个任务Give，满负荷向另一个任务Value，同时从0.8ms中断中，和高优先级任务High与Middle中，发送两个事件，一个是任务特定接收的事件，一个不是。
6 从一个任务Give，满负荷向另一个任务Value，同时从0.8ms中断中，和高优先级任务High与Middle中，发送值事件。

以上的任务优先级：High > Reactor > Sm > Value > Middle > Give
High任务发送的周期为1ms，Middle发送周期为2ms。

### TODO
1 publish和publish_time同时测试。
2 等待任务的延时如果为0
3 等待任务的延时如果为无限大
4 尚未处理等待任务的延时为无限大的情况
5 测试直接发送任务句柄的情况。看看哈希的开销到底有多大。
6 测试publish与wait_specific的结合。