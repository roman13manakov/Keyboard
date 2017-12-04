//Объявление переменных
uint8_t out_state = 0;
char out_symbol = 0;
uint8_t a = 0, b = 0, c = 0;

void init_keyboard_port(void)	//Конфигурация порта, к которому подключена клавиатура
	{
	GPIO_InitTypeDef GPIO_InitStructure;	//Структура, содержащая настройки порта
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	//Включаем тактирование порта А

	GPIO_InitStructure.GPIO_Pin = CLM_1|CLM_2|CLM_3;	//Выбираем выводы, которые будут читать состояние клавиатуры
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	//Выводы работают на вход
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	//Скорость работы выводов
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//Тип значений порта - двухуровневый
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;	//Начальное значение выводов - 0
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//Инициализация настроек

	GPIO_InitStructure.GPIO_Pin = ROW_1|ROW_2|ROW_3|ROW_4;	//Выбираем выводы, которые будут задавать высокий потенциал
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	//Выводы работают на выход
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	//Скорость работы выводов
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//Тип значений порта - двухуровневый
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;	//Начальное значение выводов - 0
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//Инициализация настроек
	}

void init_TIMs(void)	//Конфигурация таймеров	(TIM2, TIM3)
	{
	TIM_TimeBaseInitTypeDef TIM_InitStructure;	//Структура, содержащая настройки таймеров
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM3, ENABLE);	//Включаем тактирование таймеров

	TIM_InitStructure.TIM_Period = 10000;	//Период отсчета таймера
	TIM_InitStructure.TIM_Prescaler = 16;	//Предделитель частоты таймера
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//Счет таймера "вверх"
	TIM_TimeBaseInit(TIM2, &TIM_InitStructure);	//Применение прописанных настроек к таймеру 2
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);	//разрешение прерываний по таймеру 2

	TIM_TimeBaseInit(TIM3, &TIM_InitStructure);	//Применение прописанных настроек к таймеру 3
	TIM_ITConfig(TIM3,TIM_IT_Update, ENABLE);	//разрешение прерываний по таймеру 3

	TIM_Cmd(TIM2, ENABLE);	//Включение таймера 2
	TIM_Cmd(TIM3, ENABLE);	//Включение таймера 3
	TIM_TimeBaseStructInit(&TIM_InitStructure);		//Инициализация настроек
	}

void turn_on_line(uint8_t out)	//Функция, выставляющая 1 на нижней части порта клавиатуры
	{
	if (out == 1)
		{GPIO_ResetBits(GPIOA, ROW_1|ROW_2|ROW_3|ROW_4);//Сбрасываем выбранные пины в 0
		GPIO_SetBits(GPIOA, ROW_1);}									//Устанавливаем единицу на пин 1
	else if (out == 2)
		{GPIO_ResetBits(GPIOA, ROW_1|ROW_2|ROW_3|ROW_4);//Сбрасываем выбранные пины в 0
		GPIO_SetBits(GPIOA, ROW_2);}									//Устанавливаем единицу на пин 2
	else if (out == 3)
		{GPIO_ResetBits(GPIOA, ROW_1|ROW_2|ROW_3|ROW_4);//Сбрасываем выбранные пины в 0
		GPIO_SetBits(GPIOA, ROW_3);}									//Устанавливаем единицу на пин 3
	else if (out == 4)
		{GPIO_ResetBits(GPIOA, ROW_1|ROW_2|ROW_3|ROW_4);//Сбрасываем выбранные пины в 0
		GPIO_SetBits(GPIOA, ROW_4);}									//Устанавливаем единицу на пин 4

void read_input_value(void)	//Функция, сканирующая верхнюю часть порта А
	{
	a = GPIO_ReadInputDataBit(GPIOA, CLM_1);	//Читаем значение 
	b = GPIO_ReadInputDataBit(GPIOA, CLM_2);	//Читаем значение 
	c = GPIO_ReadInputDataBit(GPIOA, CLM_3);	//Читаем значение 
	}

void TIM2_IRQHandler(void)	//Обработчик, запускаемый при оставновке тамера 2
	{
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);	//Очистка состояния таймера
	switch (out_state)							//Последовательная перестановка единицы в нижней части порта А
		{
		case 0: out_state = 1;
				break;
		case 1: out_state = 2;
				break;
		case 2: out_state = 3;
				break;
		case 3: out_state = 4;
				break;
		case 4: out_state = 0;
				break;
		}
	turn_on_line(out_state);
	}


void TIM3_IRQHandler(void)	//Обработчик, запускаемый при остановке таймера 3
	{
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);	//Очистка флага состояния таймера
	read_input_value();							//Чтение данных с верхней части порта А
	if ((a>0)|(b>0)|(c>0))						//Если хотя бы одна клавиша нажата...
		{
		if (out_state == 1 && a == 1)				//Клавиша 3
			{
			out_symbol = 3;
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if	(out_state == 1 && b == 1)			//Клавиша 2
			{
			out_symbol = 2;
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if (out_state == 1 && c == 1)			//Клавиша 1
			{
			out_symbol = 1;}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if (out_state == 2 && a == 1)			//Клавиша 6
			{
			out_symbol = 6;
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if (out_state == 2 && b == 1)			//Клавиша 5
			{
			out_symbol = 5;
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if (out_state == 2 && c == 1)			//Клавиша 4
			{
			out_symbol = 4;
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if (out_state == 3 && a == 1)			//Клавиша 9
			{
			out_symbol = 9;
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if (out_state == 3 && b == 1)			//Клавиша 8
			{
			out_symbol = 8;
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if (out_state == 3 && c == 1)			//Клавиша 7
			{
			out_symbol = 7;
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if (out_state == 4 && a == 1)			//Клавиша #
			{
			out_symbol = '#';
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if (out_state == 4 && b == 1)			//Клавиша 0
			{
			out_symbol = 0;
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		else if (out_state == 4 && c == 1)			//Клавиша *
			{
			out_symbol = '*';
			}						//Передает переменной значение нажатой клавиши для дальнейшего использования
		}
	}

int main(void)
{		init_keyboard_port();	//Инициализация порта клавиатуры
    	init_TIMs();			//Инициализация таймеров
    	NVIC_EnableIRQ(TIM2_IRQn);//Включение прерываний по таймеру 2
    	NVIC_EnableIRQ(TIM3_IRQn);//Включение прерываний по таймеру 3
    while(1)
    {
    }
}


//Использование системных таймеров позволяет избежать «регистрации» дребезга контактов