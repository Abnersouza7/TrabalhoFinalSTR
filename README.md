# vTaskSoS

Trabalho final para a matéria de sistemas de tempo real, EMB-5633, onde fora implementado uma politica de escalonamento do tipo Rate Monotonic (RM), um protocolo de compartilhamento de recurso Priority Inheritance Protocol (PIP) e um servidor de tarefas aperiódicas, o Polling Server(PS).


# Arquivos necessários

Foi utilizado apenas um arquivo main.cpp onde foram feitos todos os códigos para a execução do trabalho, além disso algumas alterações foram feitas nos arquivos fonte do FreeRTOS, para a liberação de algumas funções que por padrão são bloqueadas.


# HardWare 

A equipe utilizou um Arduino UNO ligado a uma protoboard com 4 LEDs para a visualização da zona critica do sistema, além disso foi utilizado um botão para tarefa aperiódica.

# Verificação

Para a verificação se o sistema estava funcionando corretamente foi utilizado um analisador de sinal logico, Analyzer 24MHz8CH, onde por meio do software Salae, foi possível ver a ativação dos LEDs e a preempção do sistema por conta dos algoritmos utilizados

# Rate Monotonic 

Para implementar o escalonador RM se criou uma struct de dados chamada tasks que contém internamente 5 parâmetros, um ponteiro para a função de uma task, uma unidade de ticks do FreeRTOS , um char contendo o nome da task, um handler para a task já declarado anteriormente e uma variável uint8_t com a prioridade da tarefa. Após isso foi criado um vetor com três arrays dessa mesma struct,um para cada task que o sistema irá realizar. Os protótipos das tasks são definidos  e em sequência os três arrays são completos com os parâmetros inicialmente faltantes dentro deles. Com esses arrays preenchidos a equipe desenvolveu um laço de repetição for que associava todas as informações passada dentro dos arrays para a função xTaskCreate que tem quase os mesmos parâmetros. Sendo assim dentro laço for se tem a criação das tasks usando as bibliotecas do FreeRTOS que o que facilitará futuramente a sua ordenação nas filas e nos escalonadores. O objetivo dessas implementações era de conseguir declarar as tasks via as funções do FreeRTOS que facilitam o processo de escalonamento posterior, contudo com a possibilidade de se alterar o período de das mesmas. Depois de criadas as tasks tem sua prioridade definida por um algoritmo bubble-sort que define as maiores prioridades para as tarefas de menor período.

# Priority Inheritance Protocol

Para a implementação do PIP, utilizamos os semáforos disponibilizados nas bibliotecas do FreeRTOS, responsáveis pela sincronização de tarefas. Os semáforos foram do tipo Mutex que, de acordo com a documentação do FreeRTOS, faz herança de prioridade a partir da necessidade de execução, utilizando a DeadLine como um parâmetro para isto. E partindo do pressuposto que o algoritmo RM já define prioridades para as tarefas,  a implementação do PIP foi a criação desse semáforo e a utilização dele em todas as tarefas.

# Polling Server

O escalonamento de uma tarefa aperiódica atendido por um polling server consiste na abertura de um espaço na escala de tarefas periódicas através da inclusão de uma task específica para a execução da tarefa aperiódica.

# Conclusão

Foi bem divertido trabalhar com o sistema de tempo real, apesar das dificuldades que a equipe teve, muitas vezes por conta de tempo, foi possível entender bem como o sistema tem que funcionar e o porque é tão importante que um sistema funcione em tempo real. 