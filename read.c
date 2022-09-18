unsigned char read_mem(int pid, int vmem_addr) 
{
    page_table_entry* page_table_start = OS_MEM+4112*(pid-1)+16;
        int page_no= vmem_addr/PAGE_SIZE;
        int offset=vmem_addr%PAGE_SIZE;
        page_table_start+=page_no;
        page_table_entry pte=*page_table_start;
        printf("vmem add is %d",vmem_addr);
        if(is_present(pte)==1){
            int phy_frame=page_num_to_frame_num(pte);
            char*read=PS_MEM+PAGE_SIZE*phy_frame+offset;
            return *read;
        }
        else{
            return 'q';
        }
}