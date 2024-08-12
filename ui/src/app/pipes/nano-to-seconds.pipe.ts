import {Pipe, PipeTransform} from '@angular/core';

@Pipe({
  name: 'nanoToSeconds'
})
export class NanoToSecondsPipe implements PipeTransform {
  transform(value: number): { value: number } {
    return {value: Math.max(Math.round(value / 1000), 0)}
  }

}
