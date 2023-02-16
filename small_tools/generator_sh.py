
import os
#目前的port/AZ生成规则和之前的sh脚本一致
#能够修改/OPPO/run_memcached.sh
#能够修改/OPPO/run_proxy_datanode.sh
#能够修改/oppo_project/src/AZInformation.xml
current_path = os.getcwd()
parent_path = os.path.dirname(current_path)
datanode_number_per_AZ  = 20
memcached_port_start = 8000
datanode_port_start = memcached_port_start + 1000
AZid_start = 0
#考虑到最后要到集群，而proxy的ip大概可能与它AZ内的ip一致
#所以这样写proxy_ip_lis
proxy_ip_list = [
    ["0.0.0.0",50005],
    ["0.0.0.0",50015],
    ["0.0.0.0",50025],
    ["0.0.0.0",50035],
    ["0.0.0.0",50045],
    ["0.0.0.0",50055],
    ["0.0.0.0",50065],
    ["0.0.0.0",50075],
    ["0.0.0.0",50085],
    ["0.0.0.0",50095]
]
proxy_num = len(proxy_ip_list)

#目前期待的数据结构为AZ_informtion = {AZ_id:{“proxy”:0.0.0.0:50005,“datanode”:[[ip,port],...]},},酱紫的
#另外生成一个数据结构为memcached_ip_port = {AZ_id:[[ip,port],...]}
AZ_informtion = {}
memcached_ip_port = {}
def generate_AZ_info_dict():
    for i in range(proxy_num):
        new_az = {}
        
        new_az["proxy"] = proxy_ip_list[i][0]+":"+str(proxy_ip_list[i][1])
        datanode_list = []
        for j in range(datanode_number_per_AZ):
            datanode_list.append([proxy_ip_list[i][0],datanode_port_start + i*100 + j])
        new_az["datanode"] = datanode_list
        AZ_informtion[i] = new_az

    for i in range(proxy_num):
        memcached_list = []
        for j in range(datanode_number_per_AZ):
            memcached_list.append([proxy_ip_list[i][0],memcached_port_start + i*100 + j])
        memcached_ip_port[i] = memcached_list
    
def generate_run_memcached_file():
    file_name = parent_path + "/run_memcached.sh"
    with open(file_name, 'w') as f:
        f.write("kill -9 $(pidof memcached)\n")
        f.write("\n")
        for AZ_id in memcached_ip_port.keys():
            print("AZ_id",AZ_id)
            for each_datanode in memcached_ip_port[AZ_id]:
                print(each_datanode)
                f.write("./memcached/bin/memcached -m 128 -p "+str(each_datanode[1])+" --max-item-size=5242880 -vv -d\n")
            f.write("\n")
            
def generate_run_proxy_datanode_file():
    file_name = parent_path + '/run_proxy_datanode.sh'
    with open(file_name, 'w') as f:
        f.write("kill -9 $(pidof run_datanode)\n")
        f.write("kill -9 $(pidof run_proxy)\n")
        f.write("\n")
        for AZ_id in AZ_informtion.keys():
            print("AZ_id",AZ_id)
            for each_datanode in AZ_informtion[AZ_id]["datanode"]:
                f.write("./oppo_project/cmake/build/run_datanode "+str(each_datanode[0])+":"+str(each_datanode[1])+"\n")
            f.write("\n") 
        for proxy_ip_port in proxy_ip_list:
            f.write("./oppo_project/cmake/build/run_proxy "+str(proxy_ip_port[0])+":"+str(proxy_ip_port[1])+"\n")   
        f.write("\n")

def generater_AZ_information_xml():
    file_name = parent_path + '/oppo_project/config/AZInformation.xml'
    import xml.etree.ElementTree as ET
    root = ET.Element('AZs')
    root.text = "\n\t"
    for AZ_id in AZ_informtion.keys():
        az = ET.SubElement(root, 'AZ', {'id': str(AZ_id), 'proxy': AZ_informtion[AZ_id]["proxy"]})
        az.text = "\n\t\t"
        datanodes = ET.SubElement(az, 'datanodes')
        datanodes.text = "\n\t\t\t"
        for index,each_datanode in enumerate(AZ_informtion[AZ_id]["datanode"]):
            datanode = ET.SubElement(datanodes, 'datanode', {'uri': str(each_datanode[0])+":"+str(each_datanode[1])})
            #datanode.text = '\n\t\t\t'
            if index == len(AZ_informtion[AZ_id]["datanode"]) - 1:
                datanode.tail = '\n\t\t'
            else:
                datanode.tail = '\n\t\t\t'
        datanodes.tail = '\n\t'
        if AZ_id == len(AZ_informtion)-1:
            az.tail = '\n'
        else:
            az.tail = '\n\t'
    #root.tail = '\n'
    tree = ET.ElementTree(root)
    tree.write(file_name, encoding="utf-8", xml_declaration=True)

def test_chat_gpt():
    import xml.etree.ElementTree as ET
    root = ET.Element('AZs')
    az = ET.SubElement(root, 'AZ', {'id': '0', 'proxy': '0.0.0.0:50005'})
    datanodes = ET.SubElement(az, 'datanodes')
    for i in range(9000, 9020):
        datanode = ET.SubElement(datanodes, 'datanode', {'uri': '0.0.0.0:{}'.format(i)})
        datanode.tail = '\n\t\t'
    datanodes.tail = '\n\t'
    az.tail = '\n'
    tree = ET.ElementTree(root)
    tree.write('azs1.xml', encoding='utf-8', xml_declaration=True)
if __name__ == "__main__":
    generate_AZ_info_dict()
    print(AZ_informtion)
    print(memcached_ip_port)
    generate_run_memcached_file()
    generate_run_proxy_datanode_file()
    generater_AZ_information_xml()
    #test_chat_gpt()