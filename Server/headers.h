
struct req_str{
    int server_type;//请求服务的类型
    char log_tar_u_id[10];//请求聊天记录的对象id
    char log_tar_group_name[15];//请求群聊天记录的群聊名称
    char client_name[20];//请求对象的用户名
    char user_passwd[10];//用于登陆时的密码
    char user_id[10];//请求对象的ID
    char normal_meg[100];//存储普通消息存储空间
    char file_meg[1024];//存储文件消息空间
    char file_name[10];//记录文件名<用于插入文件与获取文件>
    char time_stamp[14];//消息时间辍
};

struct ack_str{
    int server_type;//确认返回服务类型
    int insert_normal_flag;//返回的数据库插入结果信号
    int insert_file_flag;//返回的数据库插入文件结果信号
    int login_flag;//确认登陆信息
    char client_name[20];//请求对象的用户名
    int user_count;//确认好友列表数量
    char user_list[20][2][15];//20个好友，两个域，名字和uid
    int group_cont;//群的数量
    int  group_mem_count[20];//每个群成员的数量
    char group_list[20][20][15];//20个群，[][0][15]表示群名，[][1][15]表示群聊id,[][2～n][15]表示群的成员名,每群最多18成员
    char client_id[10];//回复此消息记录ack的请求对象id
    char user_meglog_u_id[10];//回复此消息记录ack与请求对象聊天的对象id
    char user_meglog[100][2][50];//单人聊天消息，100条，[若为消息，0域代表消息时间，1域代表内容，每条50字节]，[若为正常消息，该域为内容，若为文件，0域代表文件名，1域为文件消息内链id]
    int user_meglog_type[100];//每条消息的数据类型，1代表(A-B)别人发的正常消息，2代表(B-A)自己发的正常消息，3代表(A-B)别人发的文件，4代表(B-A)自己发的文件
    int user_meglog_count;//记录本次从数据库获取到的信息记录数量
    char file_meg_content[1024];//返回文件消息内容
    char file_name[15];//记录文件名
};
