import { Injectable } from '@angular/core';

@Injectable({
  providedIn: 'root'
})
export class ConstantsService {

  constructor() { }

  httpServerAddress = `http://${window.location.hostname}/5000`

}
