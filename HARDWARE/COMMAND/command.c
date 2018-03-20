#include "command.h"
#include "dma.h"
#include "stdlib.h"
#include "string.h"

//��������LX200
//���ܣ�����LX200Э�飬��Э�鲨����9600��������֧��Stellarium��skysafari��
//���������buffer �����ֶ�����
//          current_pos[2] ��ǰ���꣨���룩,��������λ���㱨
//          target_pos[2] Ŀ�����꣨���룩����Ϊ���ʹ�ã����ϼ��������
//����ֵ�����ؽ���״̬��0����ʧ�ܣ�1�����ɹ����ѽ��з�����3�ɹ�����������Ŀ��RAֵ��4�ɹ�����������Ŀ��DECֵ��
//        'e'�򶫰�����'E'�򶫰���ֹͣ��'w'����������'W'��������ֹͣ��'s'���ϰ�����'S'���ϰ���ֹͣ��'n'�򱱰�����'N'�򱱰���ֹͣ
u8 LX200( u8 *buffer, s32 *current_pos, s32 *target_pos )
{
    u8 command[20], target_data[10];
    static u8 send_data[10];
    u8 i=0, a=0;

    while ( buffer[i] != ':' ) //��ͷ�������ͷʧ�ܾ�ֱ�ӷ���
    {
        i++;
        if ( i > USART_REC_LEN - 5 )
        {
            return 0;
        }
    }
    for ( a = 0; a < 19; a++ ) //ת����Ч����
    {
        command[a] = buffer[i];
        i++;
    }
    memset(buffer, 0, USART_REC_LEN); //�������

    /*ANALYZE AND REPLY*/
    if ( command[1] == 'G') //����λ���ظ���ǰ����
    {
        if ( command[2] == 'R' ) //GR �ظ��ྭ 24h60m60s
        {
            RA_ARCSEC_INTO_DEG ( current_pos[0], send_data );  //�ѵ�ǰλ�ã����룩ת��Ϊ�ȷ����ٷ��ͳ�ȥ
            DMA_SEND_DATA ( ( u32 ) send_data, 9 );
        }
        if ( command[2] == 'D' ) //GD �ظ���γ +-90:60:60
        {
            DEC_ARCSEC_INTO_DEG( current_pos[1], send_data );  //�ѵ�ǰλ�ã����룩ת��Ϊ�ȷ����ٷ��ͳ�ȥ
            DMA_SEND_DATA ( ( u32 ) send_data, 10 );
        }
        return 1;
    }

    if ( command[1] == 'Q' && command[5] == 'r' ) //#:Q#:Sr �ظ�1
    {
        target_data[0] = command[7];
        target_data[1] = command[8];
        target_data[2] = command[10];
        target_data[3] = command[11];
        target_data[4] = command[13];
        target_data[5] = command[14];
        command[0] = '1';
        DMA_SEND_DATA ( ( u32 ) command, 1 );
        target_pos[0] = RA_DEG_INTO_ARCSEC ( target_data );
        return 3;
    }

    if ( command[1] == 'Q' )
    {
        if ( command[2] == 'e' ) //Qe ң�ذ�����ֹͣ
        {
            return 'E';
        }
        if ( command[2] == 'w' ) //Qw ң�ذ�����ֹͣ
        {
            return 'W';
        }
        if ( command[2] == 's' ) //Qs ң�ذ�����ֹͣ
        {
            return 'S';
        }
        if ( command[2] == 'n' ) //Qn ң�ذ�����ֹͣ
        {
            return 'N';
        }
    }

    if ( command[1] == 'S' && command[2] == 'r' ) //:Sr �ظ�1
    {
        target_data[0] = command[3];
        target_data[1] = command[4];
        target_data[2] = command[6];
        target_data[3] = command[7];
        target_data[4] = command[9];
        target_data[5] = command[10];
        command[0] = '1';
        DMA_SEND_DATA ( ( u32 ) command, 1 );
        target_pos[0] = RA_DEG_INTO_ARCSEC ( target_data );
        return 3;
    }

    if ( command[1] == 'S' && command[2] == 'd' ) //Sd �ظ�1
    {
        if ( command[3] == ' ' )
        {
            target_data[0] = command[4];
            target_data[1] = command[5];
            target_data[2] = command[6];
            target_data[3] = command[8];
            target_data[4] = command[9];
            target_data[5] = command[11];
            target_data[6] = command[12];
        }
        else
        {
            target_data[0] = command[3];
            target_data[1] = command[4];
            target_data[2] = command[5];
            target_data[3] = command[7];
            target_data[4] = command[8];
            target_data[5] = command[10];
            target_data[6] = command[11];
        }
        command[0] = '1';
        DMA_SEND_DATA ( ( u32 ) command, 1 );
        target_pos[1] = DEC_DEG_INTO_ARCSEC ( target_data );
        return 4;
    }

    if ( command[1] == 'M' )
    {
        if ( command[2] == 'S' ) //Ms �ظ�0
        {
            command[0] = '0';
            DMA_SEND_DATA ( ( u32 ) command, 1 );
            return 1;
        }
        if ( command[2] == 'e' ) //Me ң�ذ�����
        {
            return 'e';
        }
        if ( command[2] == 'w' ) //Mw ң�ذ�����
        {
            return 'w';
        }
        if ( command[2] == 's' ) //Ms ң�ذ�����
        {
            return 's';
        }
        if ( command[2] == 'n' ) //Mn ң�ذ�����
        {
            return 'n';
        }
    }
    return 0;

}

//��������RA_STEP_CALCULATE
//���ܣ�����õ�RA���ɵ�ǰλ�õ�Ŀ��λ������Ĳ���
//���������current_ra ��ǰ��RAֵ�����룩
//          target_ra Ŀ��RAֵ�����룩
//					stp_angle RA���������Ƕȣ����룩
//����ֵ��ra_step �з���������Ϊ��Ŀ��λ������Ĳ����������Ŵ���������
int RA_STEP_CALCULATE ( s32 current_ra, s32 target_ra,double stp_angle )
{
    int ra_step ;
    if ( abs ( current_ra - target_ra ) > 648000 )  //��ֵ���ڰ���
    {
        if ( current_ra > target_ra )	//��ǰ23  Ŀ��1  ��ǰ��ֵ++
        {
            ra_step = ( target_ra + 1296000 - current_ra ) / stp_angle;
            return  ra_step;//RA��ֵ+
        }
        else 													//��ǰ1  Ŀ��23  ��ǰ��ֵ--
        {
            ra_step = ( target_ra - 1296000 - current_ra ) / stp_angle;
            return  ra_step; //RA��ֵ-
        }
    }
    else   																					 //��ֵС�ڰ���
    {
        ra_step = ( target_ra - current_ra ) / stp_angle;
        return  ra_step; //�����ٷ��������������Դ�����
    }
}

//��������DEC_STEP_CALCULATE
//���ܣ�����õ�DEC���ɵ�ǰλ�õ�Ŀ��λ������Ĳ���
//���������current_dec ��ǰ��DECֵ�����룩
//          target_dec Ŀ��DECֵ�����룩
//					stp_angle DEC���������Ƕȣ����룩
//����ֵ��dec_step �з���������Ϊ��Ŀ��λ������Ĳ����������Ŵ���������
int DEC_STEP_CALCULATE ( s32 current_dec, s32 target_dec,double stp_angle )
{
    int dec_step;
    dec_step = ( target_dec - current_dec ) / stp_angle;
    return dec_step ;
}

//��������CURRENT_POS_RA
//���ܣ���Ŀ��λ�ü�ʣ��Ĳ����Ͳ������������ǰλ��
//���������target_ra Ŀ��RAֵ�����룩
//          ra_step RAʣ��Ĳ�����
//					stp_angle RA���������Ƕȣ����룩
//����ֵ��current_ra ��ǰRA��ֵ�����룩
s32 CURRENT_POS_RA ( s32 target_ra,int ra_step,double stp_angle )
{
    s32 current_ra;
    if ( target_ra - ra_step * stp_angle < 0 ) //Ŀ��1 �ƶ�+2
    {
        current_ra = target_ra + 1296000 - ra_step * stp_angle; //1+24-2
    }
    if ( target_ra - ra_step * stp_angle > 1296000 )  //Ŀ��23 �ƶ�-2
    {
        current_ra = target_ra - 1296000 - ra_step * stp_angle; //23-24-��-2��
    }
    else																			//Ŀ��15 �ƶ�-2
    {
        current_ra = target_ra - ra_step * stp_angle;  //15-��-2��
    }
    return current_ra;
}

//��������CURRENT_POS_DEC
//���ܣ���Ŀ��λ�ü�ʣ��Ĳ����Ͳ������������ǰλ��
//���������target_dec Ŀ��DECֵ�����룩
//          dec_step DECʣ��Ĳ�����
//					stp_angle DEC���������Ƕȣ����룩
//����ֵ��current_dec ��ǰDEC��ֵ�����룩
s32 CURRENT_POS_DEC ( s32 target_dec,int dec_step, double stp_angle )
{
    s32 current_dec;
    current_dec = target_dec - dec_step * stp_angle;
    return current_dec;
}

u32 RA_DEG_INTO_ARCSEC ( u8 ra[6] ) //ra�Ķȷ���ת��Ϊ���룬��Ŀ��λ��ʱʹ��
{
    u32 result;
    result = ( ra[0] - '0' ) * 540000 + ( ra[1] - '0' ) * 54000 + ( ra[2] - '0' ) * 9000 + ( ra[3] - '0' ) * 900 + ( ra[4] - '0' ) * 150 + ( ra[5] - '0' ) * 15;
    return result;
}

u32 DEC_DEG_INTO_ARCSEC ( u8 dec[7] ) //dec�Ķȷ���ת��Ϊ���룬��Ŀ��λ��ʱʹ��
{
    u32 result;
    result = ( dec[1] - '0' ) * 36000 + ( dec[2] - '0' ) * 3600 + ( dec[3] - '0' ) * 600 + ( dec[4] - '0' ) * 60 + ( dec[5] - '0' ) * 10 + dec[6] - '0';
    if ( dec[0] == '+' )
        result = 324000 + result;
    if ( dec[0] == '-' )
        result = 324000 - result;
    return result;
}

void RA_ARCSEC_INTO_DEG( s32 current_ra, u8 *ra_out ) //����current_ra���µ�ǰ����ֵ���ظ�QD QRʱʹ��
{
    s32 temp;
    temp = current_ra / 54000;
    ra_out[0] = ( temp / 10 ) + '0';
    ra_out[1] = ( temp % 10 ) + '0';
    ra_out[2] = ':';

    temp = current_ra % 54000;
    temp = temp / 900;
    ra_out[3] = ( temp / 10 ) + '0';
    ra_out[4] = ( temp % 10 ) + '0';
    ra_out[5] = ':';

    temp = current_ra % 900;
    temp = temp / 15;
    ra_out[6] = ( temp / 10 ) + '0';
    ra_out[7] = ( temp % 10 ) + '0';
    ra_out[8] = '#';
}

void DEC_ARCSEC_INTO_DEG ( s32 current_dec, u8 *dec_out ) //����current_dec���µ�ǰ����ֵ���ظ�QD QRʱʹ��
{
    s32 temp;
    s32 temp2;
    if ( current_dec >= 324000 )
    {
        temp2 = current_dec;
        temp2 = temp2 - 324000;
        dec_out[0] = '+';
    }
    if ( current_dec < 324000 )
    {
        temp2 = current_dec;
        temp2 = 324000 - temp2;
        dec_out[0] = '-';
    }

    temp = temp2 / 3600;
    dec_out[1] = ( temp / 10 ) + '0';
    dec_out[2] = ( temp % 10 ) + '0';
    dec_out[3] = ':';

    temp = temp2 % 3600;
    temp = temp / 60;
    dec_out[4] = ( temp / 10 ) + '0';
    dec_out[5] = ( temp % 10 ) + '0';
    dec_out[6] = ':';

    temp = temp2 % 60;
    dec_out[7] = ( temp / 10 ) + '0';
    dec_out[8] = ( temp % 10 ) + '0';
    dec_out[9] = '#';
}

u8 GOTO_CHECK( u8 decode_state)
{
    static u8 goto_flag=0;
    if ( goto_flag==0xff) goto_flag=0;
    if(decode_state==3)
    {
        goto_flag|=0xf0;
    }
    if(decode_state==4)
    {
        goto_flag|=0x0f;
    }
    return goto_flag;
}

void REMOTE_KEY_CONTROL ( s8 *remote_key_state,  u8 decode_state )
{
    switch (decode_state)
    {
    case 'E':
        remote_key_state[0]=0;
        break;
    case 'W':
        remote_key_state[0]=0;
        break;
    case 'S':
        remote_key_state[1]=0;
        break;
    case 'N':
        remote_key_state[1]=0;
        break;
    case 'e':  //RA-
        remote_key_state[0]=1;
        break;
    case 'w':  //RA+
        remote_key_state[0]=-1;
        break;
    case 's':  //DEC-
        remote_key_state[1]=-1;
        break;
    case 'n':  //DEC+
        remote_key_state[1]=1;
        break;
    default:
        break;
    }
}

void SHUTTER_CONTROL( u8 *buffer, u16 *shutter )
{
	char temp[5]={0};
	u8 i=0 , j=0; 
	if( buffer[0]=='T' && buffer[1]=='p') //Э����ͷ
	{
		while(buffer[i+2]!=' ')  //��һ������ת��
		{
			temp[i]=buffer[i+2];
			i++;
		}
		shutter[0]  = atof(temp); //תΪu16
		
		memset(temp, 0, 5); //�������
		i++;
		while(buffer[i+2]!=0)  //�ڶ�������ת��
		{
			temp[j]=buffer[i+2];
			i++;
			j++;
		}
		shutter[1] = atof(temp);
		memset(buffer, 0, 20); //�������
	}
}

