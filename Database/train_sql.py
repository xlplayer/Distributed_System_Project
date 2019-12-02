import pymysql
conn = pymysql.connect(host='127.0.0.1', user = "root", passwd="1234", db="train", port=3306, charset="utf8")
cur = conn.cursor()
#sql语句
sql = "insert into station_train(start, end, train) value(%s, %s, %s)"
#数据 
data = {
        "G102":["上海虹桥","无锡东","常州北","南京南", "滁州", "蚌埠南", "宿州东", "徐州东", "济南西", "廊坊", "北京南"],
        "G6":["上海", "苏州北", "南京南", "济南西", "北京南"],
        "Z282":["杭州", "海宁", "嘉兴", "上海南", "苏州", "无锡", "常州", "镇江", "南京", "蚌埠", "徐州", "兖州", "德州", "天津西"
                "北京", "张家口南", "大同", "集宁南", "呼和浩特东", "呼和浩特", "包头东", "包头"]
        }
for key in data:
    vals = data[key]
    print(key,end=": ")
    for val in vals:
        print(val,end=" ")
    print()

for key in data:
    vals = data[key]
    for i in range(len(vals)):
        a = vals[i]
        for j in range(i+1, len(vals)):
            b = vals[j]
            param = tuple([a,b,key])
            cur.execute(sql, param)

#提交事务
conn.commit()

#关闭资源连接
cur.close()
conn.close()
print("数据库断开连接！")
