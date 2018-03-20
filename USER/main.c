#include "delay.h"
#include "sys.h"
#include "dma.h"
#include "timer.h"
#include "EQ3D.h"
#include "usart.h"
#include "command.h"

u8 data_receive_buffer[USART_REC_LEN];  //�����ڴ���DMA����
u8 step_flag=0;  //��ʱ���жϱ�־

int main(void)
{
    s32 current_pos[2]= {0,0}, target_pos[2]= {0,0};
    s32 target_ra=0, target_dec=0;
    int ra_step=0, dec_step=0;
    u8 decode_state=0, dir_state[2]= {0,0} ;
    u16 move_speed=1;
    s8 remote_key_state[2]= {0,0}, local_key_state[2]= {0,0}, key_state[2]= {0,0} ;
		u16 shutter[2]={0,0}, timer_counter=0 , sec_counter=0 ;

    delay_init();	       //��ʱ������ʼ��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
    uart3_init(9600);
    DMA_TX_init((u32)data_receive_buffer, USART_REC_LEN);
    DMA_RX_init((u32)data_receive_buffer, USART_REC_LEN);
    EQ3D_GPIO_KEY_Config();
    EQ3D_GPIO_OUT_Config();
    TIM3_Int_Init(STP_INTERVAL, 35); //42�������3595+1΢��/�ģ�
    PWR_LED = 1; //��Դָʾ��

    while(1)
    {
        if(data_receive_buffer[USART_REC_LEN - 1] == 1)  //DMA�յ���Ϣ��־
        {
            data_receive_buffer[USART_REC_LEN - 1] = 0; //�����־
					
						SHUTTER_CONTROL( data_receive_buffer, shutter); //��������߿���
					
            decode_state = LX200( data_receive_buffer, current_pos, target_pos ); //LX200Э�����
            REMOTE_KEY_CONTROL(remote_key_state,decode_state); //���߰������ƽ���
            if(GOTO_CHECK(decode_state) == 0xff) //Ŀ��λ���뱸
            {
                if(current_pos[0] == 0 && current_pos[1] == 0) //�����ϵͳ�տ�ʼ���У��򽫵�һ�ε�GOTO��Ϣ����Ϊ��ǰλ��
                {
                    current_pos[0] = target_pos[0];
                    current_pos[1] = target_pos[1];
                }
                else  //����GOTOʱ������Ŀ��λ�ü��㲽�������򣬸��ݲ����ͷ������ϵ���õ�ʵ����Ҫ�Ĳ����ͷ���
                {
                    target_ra = target_pos[0];
                    target_dec = target_pos[1];
                    ra_step	=	RA_STEP_CALCULATE(current_pos[0], target_pos[0], RA_STP_ANGLE );   //����������貽���ͷ��������Ŵ�����
                    dec_step	=	DEC_STEP_CALCULATE(current_pos[1], target_pos[1], DEC_STP_ANGLE );

                    if(ra_step>0)
                    {
                        ra_step=ra_step*((float) NORM_STP_COUNT/(float)(NORM_STP_COUNT+GOTO_STP_COUNT));
                    }
                    else
                    {
                        ra_step=ra_step*((float) NORM_STP_COUNT/(float)(NORM_STP_COUNT-GOTO_STP_COUNT));
                    }
                }
            }
        }
        if(step_flag == 1)  //������־
        {
            step_flag = 0;  //�����־
            DIR_CONTROL(dir_state);  //�����л�������ƶ������׼ֵ���������򣬱����������ϰ������ϰ���������Ϊ�˽�����췭ת���ϱ��������õ�����
            if(ra_step == 0 && dec_step == 0)  //����GOTO
            {
                LOCAL_KEY_CONTROL(local_key_state); //�ֱ����Ƴ�����ƶ�
                KEY_CONTROL_MIX(remote_key_state,local_key_state,key_state);  //�ֱ���ң�ذ��������ں�
                move_speed=SPEED_CONTROL();  //����ʱ������ƶ��ٶ�����
                HANDLE_CONTROL( move_speed,dir_state,key_state);  //��Ӧ�����������ٶȸ���
            }
            else  //��ҪGOTO
            {
                GOTO( &ra_step, &dec_step,dir_state);    //ִ��GOTO����
                current_pos[0] = CURRENT_POS_RA ( target_ra, ra_step, RA_STP_ANGLE );   //���µ�ǰָ��
                current_pos[1] = CURRENT_POS_DEC ( target_dec, dec_step, DEC_STP_ANGLE );
            }
						timer_counter++;
        }
				
				if(shutter[1]!=0&&timer_counter>=30769)//���ſ���
				{
					timer_counter=0;
					sec_counter++;
					if(sec_counter==5&&shutter[0]!=0) //��ʱ5���B������
					{
						PCout(13)=0;
						PBout(12)=0;
					}
					if(sec_counter>=(shutter[0]+5))  //��ʱֹͣB��
					{
						PCout(13)=1;
						PBout(12)=1;
						shutter[1]--;
						sec_counter=0;
					}
				}
    }
}

